#include <iostream>
#include <list>
#include <string>

#include <DefaultProperty.h>

int main()
{
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
		const std::function<bool(PersonModel&)> OnPreNameChange = [](PersonModel& PersonData) -> bool
		{
			std::cout << "Pre name change : " << PersonData.ToString() << "\n";

			return true;
		};

		const std::function<bool(PersonModel&)> OnPostNameChange = [](PersonModel& PersonData) -> bool
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
			const DataBinding::PropertyBase<PersonModel>::ECommandStatus CommandStatus = PersonProperty.RunCommand([&NewFirstName, &NewLastName, this](PersonModel& PersonData) -> bool
			{
				PersonData.SetFirstName(NewFirstName);
				PersonData.SetLastName(NewLastName);

				return true;
			});

			std::cout << "Command status : " << (int)CommandStatus << "\n";
		}
	};

	PersonModel PersonData;
	PersonViewModel PersonBindings(PersonData);
	PersonView PersonUI;

	constexpr bool ExecuteOnSubscribe = true;

	auto PreTransformHandle = PersonBindings.PersonProperty.SubscribePreTransform(PersonUI.OnPreNameChange, ExecuteOnSubscribe);
	PersonBindings.PersonProperty.SubscribePostTransform(PersonUI.OnPostNameChange, !ExecuteOnSubscribe);

	PersonBindings.RunChangeNameCommand("NewFirstName", "NewLastName");

	PersonBindings.PersonProperty.Unsubscribe(PreTransformHandle);

	PersonBindings.RunChangeNameCommand("NewestFirstName", "NewestLastName");

    std::cout << "END\n";
}