#pragma once

#include "PropertyBase.h"

#include <algorithm>
#include <cassert>
#include <list>

namespace DataBinding
{

template<class BoundDataType>
class DefaultProperty final : public PropertyBase<BoundDataType>
{
	using SubscriberHandleToDelegateType = std::pair<PropertySubscriberHandle, std::function<bool(BoundDataType&)>>;

public:
	DefaultProperty(BoundDataType& BoundDataValue) : PropertyBase<BoundDataType>(BoundDataValue) {}

	PropertySubscriberHandle SubscribePreTransform(std::function<bool(BoundDataType&)> Delegate) override
	{
		return Subscribe(Delegate, PreOrderedSubscriberHandlesToDelegates);
	}

	PropertySubscriberHandle SubscribePostTransform(std::function<bool(BoundDataType&)> Delegate) override
	{
		return Subscribe(Delegate, PostOrderedSubscriberHandlesToDelegates);
	}

	bool Unsubscribe(PropertySubscriberHandle SubscriberHandle) override
	{
		if (SubscriberHandle.IsSubscribed())
		{
			assert(IsValid(SubscriberHandle));

			auto SubscriberHandlesToDelegatesPredicate = [SubscriberHandle](const SubscriberHandleToDelegateType& SubscriberHandleToDelegate)
			{
				assert(CurrentId >= 0);

				return SubscriberHandleToDelegate.first == SubscriberHandle;
			};

			auto SubscriberHandlesToDelegatesIterator = std::find_if(PostOrderedSubscriberHandlesToDelegates.cbegin(), PostOrderedSubscriberHandlesToDelegates.cend(), SubscriberHandlesToDelegatesPredicate);

			if (SubscriberHandlesToDelegatesIterator != PostOrderedSubscriberHandlesToDelegates.cend())
			{
				PostOrderedSubscriberHandlesToDelegates.erase(SubscriberHandlesToDelegatesIterator);
			}
			else
			{
				SubscriberHandlesToDelegatesIterator = std::find_if(PreOrderedSubscriberHandlesToDelegates.cbegin(), PreOrderedSubscriberHandlesToDelegates.cend(), SubscriberHandlesToDelegatesPredicate);

				assert(SubscriberHandlesToDelegatesIterator != PreOrderedSubscriberHandlesToDelegates.cend());

				PreOrderedSubscriberHandlesToDelegates.erase(SubscriberHandlesToDelegatesIterator);
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	void UnsubscribeAll() override
	{
		PreOrderedSubscriberHandlesToDelegates.clear();
		PostOrderedSubscriberHandlesToDelegates.clear();
	}

	bool IsValid(PropertySubscriberHandle SubscriberHandle) const override
	{
		if (SubscriberHandle.IsSubscribed())
		{
			auto SubscriberHandlesToDelegatesPredicate = [SubscriberHandle](const SubscriberHandleToDelegateType& SubscriberHandleToDelegate)
			{
				assert(CurrentId >= 0);

				return SubscriberHandleToDelegate.first == SubscriberHandle;
			};

			return
				std::find_if(PreOrderedSubscriberHandlesToDelegates.cbegin(), PreOrderedSubscriberHandlesToDelegates.cend(), SubscriberHandlesToDelegatesPredicate) != PreOrderedSubscriberHandlesToDelegates.cend() ||
				std::find_if(PostOrderedSubscriberHandlesToDelegates.cbegin(), PostOrderedSubscriberHandlesToDelegates.cend(), SubscriberHandlesToDelegatesPredicate) != PostOrderedSubscriberHandlesToDelegates.cend();
		}
		else
		{
			return false;
		}
	}

protected:
	bool OnPreTransform(BoundDataType& BoundData) override
	{
		for (const auto& SubscriberHandleToDelegate : PreOrderedSubscriberHandlesToDelegates)
		{
			if (!SubscriberHandleToDelegate.second(BoundData))
			{
				return false;
			}
		}

		return true;
	}

	bool OnPostTransform(BoundDataType& BoundData) override
	{
		for (const auto& SubscriberHandleToDelegate : PostOrderedSubscriberHandlesToDelegates)
		{
			if (!SubscriberHandleToDelegate.second(BoundData))
			{
				return false;
			}
		}

		return true;
	}

private:
	PropertySubscriberHandle Subscribe(std::function<bool(BoundDataType&)> Delegate, std::list<SubscriberHandleToDelegateType>& OrderedSubscriberHandlesToDelegates)
	{
		{
			assert(CurrentId >= 0);

			PropertySubscriberHandle PotentialSubscriberHandle{ CurrentId };
			auto SubscriberHandlesToDelegatesPredicate = [&PotentialSubscriberHandle](const SubscriberHandleToDelegateType& SubscriberHandleToDelegate)
			{
				return SubscriberHandleToDelegate.first == PotentialSubscriberHandle;
			};

			while (
				std::find_if(PostOrderedSubscriberHandlesToDelegates.cbegin(), PostOrderedSubscriberHandlesToDelegates.cend(), SubscriberHandlesToDelegatesPredicate) != PostOrderedSubscriberHandlesToDelegates.cend() ||
				std::find_if(PreOrderedSubscriberHandlesToDelegates.cbegin(), PreOrderedSubscriberHandlesToDelegates.cend(), SubscriberHandlesToDelegatesPredicate) != PreOrderedSubscriberHandlesToDelegates.cend()
				)
			{
				CurrentId = std::max(++CurrentId, 0);
				PotentialSubscriberHandle = { CurrentId };
			}
		}

		OrderedSubscriberHandlesToDelegates.emplace_back(SubscriberHandleToDelegateType{ CurrentId, Delegate });

		return OrderedSubscriberHandlesToDelegates.back().first;
	}

	std::list<SubscriberHandleToDelegateType> PreOrderedSubscriberHandlesToDelegates;
	std::list<SubscriberHandleToDelegateType> PostOrderedSubscriberHandlesToDelegates;

	static int CurrentId;
};

template<class BoundDataType>
int DefaultProperty<BoundDataType>::CurrentId = 0;
}

