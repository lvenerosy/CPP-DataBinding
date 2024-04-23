#pragma once

#include <functional>
#include <memory>

namespace DataBinding
{

static nullptr_t NullContext;

class PropertySubscriberHandle final
{
public:
	PropertySubscriberHandle(int IdValue) : Id(IdValue) {}

	bool IsSubscribed() const { return Id >= 0; }

	PropertySubscriberHandle& operator=(const PropertySubscriberHandle& Other)
	{
		Id = Other.Id;

		return *this;
	}

	inline friend bool operator==(const PropertySubscriberHandle& LHS, const PropertySubscriberHandle& RHS) { return LHS.Id == RHS.Id; }
	inline friend bool operator!=(const PropertySubscriberHandle& LHS, const PropertySubscriberHandle& RHS) { return !(LHS == RHS); }

	inline friend bool operator<(const PropertySubscriberHandle& LHS, const PropertySubscriberHandle& RHS) { return LHS.Id < RHS.Id; }

private:
	int Id = -1;
};

template<class BoundDataType, typename ContextType = nullptr_t>
class PropertyBase
{
public:
	// The context is supposed to be a reference to a struct aggregating the relevant data for all possible transformations,
	// the alternative would be to use a pointer and allow polymorphism but that would encourage using dynamic cast
	// Instead of aggregating, it is possible (better ?) to define one data binding per transformation type each with a unique context,
	// this way allows for subscribing to only the desired transformations as well
	using TransformerType = std::function<bool(BoundDataType&, ContextType&)>;

	enum class ECommandStatus : int
	{
		UNBOUND = 0,
		PRE_TRANSFORM_FAILURE,
		TRANSFORM_FAILURE,
		POST_TRANSFORM_FAILURE,
		SUCCESS
	};

	PropertyBase(BoundDataType& BoundDataValue) : BoundData(&BoundDataValue) {}
	virtual ~PropertyBase() = default;

	PropertySubscriberHandle SubscribePreTransform(TransformerType Delegate)
	{
		return OnSubscribePreTransform(Delegate);
	}

	PropertySubscriberHandle ExecuteAndSubscribePreTransform(TransformerType Delegate, ContextType& Context)
	{
		Delegate(*BoundData, Context);

		return OnSubscribePreTransform(Delegate);
	}

	PropertySubscriberHandle SubscribePostTransform(TransformerType Delegate)
	{
		return OnSubscribePostTransform(Delegate);
	}

	PropertySubscriberHandle ExecuteAndSubscribePostTransform(TransformerType Delegate, ContextType& Context)
	{
		Delegate(*BoundData, Context);

		return OnSubscribePostTransform(Delegate);
	}

	virtual bool Unsubscribe(PropertySubscriberHandle SubscriberHandle) = 0;
	virtual void UnsubscribeAll() = 0;

	// Implement to check that PropertyBase::Transform would return true if called
	// Most likely slow so use it for asserts, instead use PropertySubscriberHandle::IsSubscribed
	virtual bool IsValid(PropertySubscriberHandle SubscriberHandle) const = 0;

	bool IsBound() const { return BoundData; }

	// Returns a status so you can assert that a particular transform should never fail pre transform for example
	ECommandStatus RunCommand(TransformerType Transfomer, ContextType& Context)
	{
		if (!IsBound())
		{
			return ECommandStatus::UNBOUND;
		}
		else if (!OnPreTransform(*BoundData, Context))
		{
			return ECommandStatus::PRE_TRANSFORM_FAILURE;
		}
		else if (!Transfomer(*BoundData, Context))
		{
			return ECommandStatus::TRANSFORM_FAILURE;
		}
		else if (!OnPostTransform(*BoundData, Context))
		{
			return ECommandStatus::POST_TRANSFORM_FAILURE;
		}
		else
		{
			return ECommandStatus::SUCCESS;
		}
	}

protected:
	virtual PropertySubscriberHandle OnSubscribePreTransform(TransformerType Delegate) = 0;
	virtual PropertySubscriberHandle OnSubscribePostTransform(TransformerType Delegate) = 0;

	virtual bool OnPreTransform(BoundDataType& BoundData, ContextType& Context) = 0;
	virtual bool OnPostTransform(BoundDataType& BoundData, ContextType& Context) = 0;

private:
	BoundDataType* const BoundData = nullptr;
};

}