# C++ data binding library

## MVVM example

```cpp
class PersonModel
{
public:
	std::string ToString() const { return FirstName + " " + LastName; }

	void SetFirstName(const std::string& NewFirstName) { FirstName = NewFirstName; }
	void SetLastName(const std::string& NewLastName) { LastName = NewLastName; }

private:
	std::string FirstName = "OldFirstName";
	std::string LastName = "OldLastName";
};

class PersonView
{
public:
	const DataBinding::PropertyBase<PersonModel>::TransformerType OnPreNameChange = [](PersonModel& PersonData, nullptr_t&) -> bool
	{
		std::cout << "Pre name change : " << PersonData.ToString() << "\n";

		return true;
	};

	const DataBinding::PropertyBase<PersonModel>::TransformerType OnPostNameChange = [](PersonModel& PersonData, nullptr_t&) -> bool
	{
		std::cout << "Post name change : " << PersonData.ToString() << "\n";

		return true;
	};
};

class PersonViewModel
{
public:
	PersonViewModel(PersonModel& PersonData) : PersonProperty(PersonData) {}

	// Could be done for each member variable of PersonModel instead
	DataBinding::DefaultProperty<PersonModel> PersonProperty;

	void RunChangeNameCommand(const std::string& NewFirstName, const std::string& NewLastName)
	{
		const DataBinding::PropertyBase<PersonModel>::ECommandStatus CommandStatus = PersonProperty.RunCommand([&NewFirstName, &NewLastName, this](PersonModel& PersonData, nullptr_t&) -> bool
		{
			PersonData.SetFirstName(NewFirstName);
			PersonData.SetLastName(NewLastName);

			return true;
		}, DataBinding::NullContext);

		std::cout << "Command status : " << (int)CommandStatus << "\n";
	}
};

std::cout << "BEGIN MVVM EXAMPLE\n";

PersonModel PersonData;
PersonViewModel PersonBindings(PersonData);
PersonView PersonUI;

constexpr bool ExecuteOnSubscribe = true;

auto PreTransformHandle = PersonBindings.PersonProperty.ExecuteAndSubscribePreTransform(PersonUI.OnPreNameChange, DataBinding::NullContext);
PersonBindings.PersonProperty.SubscribePostTransform(PersonUI.OnPostNameChange);

PersonBindings.RunChangeNameCommand("NewFirstName", "NewLastName");

PersonBindings.PersonProperty.Unsubscribe(PreTransformHandle);

PersonBindings.RunChangeNameCommand("NewestFirstName", "NewestLastName");

std::cout << "END MVVM EXAMPLE\n";
```

Output

```
BEGIN MVVM EXAMPLE
Pre name change : OldFirstName OldLastName
Pre name change : OldFirstName OldLastName
Post name change : NewFirstName NewLastName
Command status : 4
Post name change : NewestFirstName NewestLastName
Command status : 4
END MVVM EXAMPLE
```

## Vector binding with context example

```cpp
struct VectorContext
{
	int TargetIndex = -1;
};

const DataBinding::PropertyBase<std::vector<int>, VectorContext>::TransformerType OnPreInsertOrPushback = [](std::vector<int>& Values, VectorContext& Context) -> bool
{
	std::cout << "Current value at index : ";

	(Context.TargetIndex >= 0 && Context.TargetIndex < Values.size()) ?
		std::cout << Values[Context.TargetIndex] :
		std::cout << "Index not in range, will pushback instead";
	std::cout << "\n";

	return true;
};

const DataBinding::PropertyBase<std::vector<int>, VectorContext>::TransformerType OnPostInsertOrPushback = [](std::vector<int>& Values, VectorContext& Context) -> bool
{
	std::cout << "New value " << Values.at(Context.TargetIndex) << " at index " << Context.TargetIndex << "\n";

	return true;
};

std::cout << "BEGIN CONTEXT EXAMPLE\n";

VectorContext InsertionContext;

std::vector<int> Values{ 1, 2, 3, 4 };
const int ValueToAdd = 5;

DataBinding::DefaultProperty<std::vector<int>, VectorContext> ValuesProperty(Values);

ValuesProperty.SubscribePreTransform(OnPreInsertOrPushback);
ValuesProperty.SubscribePostTransform(OnPostInsertOrPushback);

ValuesProperty.RunCommand([ValueToAdd](std::vector<int>& Values, VectorContext& Context) -> bool
{
	if (Context.TargetIndex >= 0 && Context.TargetIndex < Values.size())
	{
		Values.insert(Values.cbegin() + Context.TargetIndex, ValueToAdd);
	}
	else
	{
		Context.TargetIndex = (int)Values.size();
		Values.push_back(ValueToAdd);
	}

	return true;
}, InsertionContext);

std::cout << "END CONTEXT EXAMPLE\n";
```

Output

```
BEGIN CONTEXT EXAMPLE
Current value at index : Index not in range, will pushback instead
New value 5 at index 4
END CONTEXT EXAMPLE
```