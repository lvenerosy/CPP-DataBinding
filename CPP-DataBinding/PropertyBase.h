#pragma once

#include <functional>

namespace DataBinding
{

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

template<class BoundDataType>
class PropertyBase
{
public:
	enum class ECommandStatus : int
	{
		UNBOUND = 0,
		PRE_TRANSFORM_FAILURE,
		TRANSFORM_FAILURE,
		POST_TRANSFORM_FAILURE,
		SUCCESS
	};

	PropertyBase(BoundDataType& BoundDataValue) : BoundData(&BoundDataValue) {}
	virtual ~PropertyBase() {}

	virtual PropertySubscriberHandle SubscribePreTransform(std::function<bool(BoundDataType&)> Delegate) = 0;
	virtual PropertySubscriberHandle SubscribePostTransform(std::function<bool(BoundDataType&)> Delegate) = 0;

	virtual bool Unsubscribe(PropertySubscriberHandle SubscriberHandle) = 0;
	virtual void UnsubscribeAll() = 0;

	// Implement to check that PropertyBase::Transform would return true if called
	// Most likely slow so use it for asserts, instead use PropertySubscriberHandle::IsSubscribed
	virtual bool IsValid(PropertySubscriberHandle SubscriberHandle) const = 0;

	bool IsBound() const { return BoundData; }

	// Returns a status so you can assert that a particular transform should never fail pre transform for example
	ECommandStatus RunCommand(std::function<bool(BoundDataType&)> Transfomer)
	{
		if (!IsBound())
		{
			return ECommandStatus::UNBOUND;
		}
		else if (!OnPreTransform(*BoundData))
		{
			return ECommandStatus::PRE_TRANSFORM_FAILURE;
		}
		else if (!Transfomer(*BoundData))
		{
			return ECommandStatus::TRANSFORM_FAILURE;
		}
		else if (!OnPostTransform(*BoundData))
		{
			return ECommandStatus::POST_TRANSFORM_FAILURE;
		}
		else
		{
			return ECommandStatus::SUCCESS;
		}
	}

protected:
	virtual bool OnPreTransform(BoundDataType& BoundData) = 0;
	virtual bool OnPostTransform(BoundDataType& BoundData) = 0;

private:
	BoundDataType* const BoundData = nullptr;
};

}