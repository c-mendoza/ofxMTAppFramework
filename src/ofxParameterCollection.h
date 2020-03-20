//
// Created by Cristobal Mendoza on 3/16/20.
//

#ifndef NERVOUSSTRUCTUREOF_ParameterCollection_HPP
#define NERVOUSSTRUCTUREOF_ParameterCollection_HPP

#include <ofParameter.h>

class EventListenerStorage
{
public:
	void addListener(std::shared_ptr<ofAbstractParameter> param, std::unique_ptr<of::priv::AbstractEventToken>&& listener)
	{
		listenerStorage.insert({param, std::move(listener)});
	}

	void removeListenersForParam(std::shared_ptr<ofAbstractParameter> param) {
		listenerStorage.erase(param);
	}

	void removeAll() {
		listenerStorage.clear();
	}

private:
	std::unordered_multimap<std::shared_ptr<ofAbstractParameter>,
	        std::unique_ptr<of::priv::AbstractEventToken>> listenerStorage;
};

template<typename ParameterType>
class ofxParameterCollection
{
protected:

	std::string itemPrefix;
	ofParameterGroup parameterGroup;
	bool isSetup = false;
	std::vector<std::shared_ptr<ofParameter<ParameterType>>> parameters;
	ofEventListeners valueListeners;
private:


public:

	ofEvent<ofxParameterCollection<ParameterType>> collectionChangedEvent;
	ofEvent<ofParameter<ParameterType>> collectionItemChangedEvent;

	void setup(std::string itemPrefix, std::string groupName)
	{
		this->itemPrefix = itemPrefix;
		parameterGroup.setName(groupName);
		isSetup = true;
	}

	void addItem(ParameterType& value, bool notify = true)
	{
		ofParameter<ParameterType> param;
		param.set(itemPrefix + ofToString(parameterGroup.size()),
				  value);
		auto paramPtr = std::make_shared<ofParameter<ParameterType>>(param);

		valueListeners.push(paramPtr->newListener([&, paramPtr](ParameterType& value)
												  {
													  collectionItemChangedEvent.notify(*paramPtr);
												  }));

		parameters.push_back(paramPtr);
		parameterGroup.add(*paramPtr);
		if (notify) collectionChangedEvent.notify(*this);
	}

	/**
	 * @brief Adds an entry to the collection. The value for the entry's parameter is null. This
	 * is mostly useful to get the ParameterGroup ready for deserialization.
	 * @param notify determines whether the collectionChangedEvent fires. The default is true.
	 */
	void addEntry(bool notify = true)
	{
		ParameterType value;
		addItem(value, notify);
	}

	/**
	 * @brief Adds multiple entries with null parameter values. This
	 * is mostly useful to get the ParameterGroup ready for deserialization.
	 * @param count
	 * @param notify
	 */
	void addEntries(int count, bool notify = true)
	{
		for (int i = 0; i < count; i++)
		{
			addEntry(notify);
		}
	}

	void addItemAt(ofAbstractParameter& param, int i, bool notify = true)
	{}

	ofParameter<ParameterType>& getAt(int i)
	{
		parameters.at(i);
	}

	void deleteAt(int i, bool notify = true)
	{

	}

	void removeItem(ofParameter<ParameterType> param, bool notify = true)
	{
		auto iter = std::find(parameters.begin(), parameters.end(), param);
		if (iter != parameters.end())
		{
			parameterGroup.remove(param);
			parameters.erase(iter);
			if (notify) collectionChangedEvent.notify(*this);
			assert(parameterGroup.size() == parameters.size());
		}
	}

	/**
	 * @brief Clears and rebuilds the ParameterCollection using the supplied values.
	 * All parameters previously in the collection are removed, so make sure that you are not
	 * relying on listening for value changes in individual parameters.
	 * The listeners to the collectionChangedEvent are not affected.
	 * @param newCollection
	 */
	void setCollection(std::vector<std::shared_ptr<ParameterType>> newCollection)
	{
		this->clear(false);
		for (auto& paramPtr : newCollection)
		{
			addItem(*paramPtr, false);
		}
		notify();
	}

	/**
	 * @brief Clears and rebuilds the ParameterCollection using the supplied values.
	 * All parameters previously in the collection are removed, so make sure that you are not
	 * relying on listening for value changes in individual parameters.
	 * The listeners to the collectionChangedEvent are not affected.
	 * @param newCollection
	 */
	void setCollection(std::vector<ParameterType> newCollection)
	{
		this->clear(false);
		for (auto& value : newCollection)
		{
			addItem(value, false);
		}
		notify();
	}

	/**
	 * @brief Sets the values of the ofParameters in the collection. The @param newValues size() must be
	 * equal to the number of ofParameters currently in the collection.
	 * @param newValues
	 */
	void setValues(std::vector<std::shared_ptr<ParameterType>> newValues)
	{
		assert(newValues.size() == parameters.size());
		for (int i = 0; i < parameters.size(); i++)
		{
			parameters[i].get() = *(newValues[i]);
		}
		notify();
	}

	/**
	 * @brief Sets the values of the ofParameters in the collection. The @param newValues size() must be
	 * equal to the number of ofParameters currently in the collection.
	 * @param newValues
	 */
	void setValues(std::vector<ParameterType> newValues)
	{
		assert(newValues.size() == parameters.size());
		for (int i = 0; i < parameters.size(); i++)
		{
			parameters[i].get() = newValues[i];
		}
		notify();
	}

	typename std::vector<std::shared_ptr<ofParameter<ParameterType>>>::const_iterator begin()
	{
		return parameters.begin();
	}

	typename std::vector<std::shared_ptr<ofParameter<ParameterType>>>::const_iterator end()
	{
		return parameters.end();
	}

	void clear(bool notify = true)
	{
		// I'm not calling parameterGroup.clear() because I was concerned that it would clear
		// its value pointer and muck up the Parameter tree elsewhere. Testing would clear that up, but
		// this works for the moment
		for (int i = parameterGroup.size() - 1; i >= 0; i--)
		{
			parameterGroup.remove(i);
		}
		parameters.clear();
		valueListeners.unsubscribeAll();
		if (notify) collectionChangedEvent.notify(*this);
	}

	/**
	 * @brief Gets you the ofParameterGroup.
	 * @return What I just said it does.
	 */
	ofParameterGroup& getGroup()
	{
		assert(isSetup);
		return parameterGroup;
	}

	/**
	 * @brief Call this method prior to deserializing your ofParameterGroup.
	 * Why do we need to pre-deserialize?
	 * OF deserializes by loading a serialized file and then trying to match tag names to Parameter names. This
	 * works great when you know all of your parameters ahead of time, but when we are working with collections,
	 * by definition, we don't know how many elements we'll end up having, and thus we can't know the names of
	 * the parameters we are trying to match. What this method does is to look into the XML file and find how
	 * many XML elements with the specified prefix are in an element with the name of our parameterGroup. It will then
	 * insert into the collection that number of ofParameters, naming them sequentially with our prefix, adding them
	 * also to the parameterGroup. Once we have properly named parameters in the group that match the XML file, we
	 * can proceed to deserialize. Not that when you predeserialize, the created parameters have null values. You
	 * should call ofDeserialize right after preDeserialize.
	 *
	 * @param xml The XML root from which to start searching for the parameter group.
	 * @param clear True by default. Clears the parameter group and the underlying data storage prior to
	 * pre-deserialization. Not sure why you would not want to clear the backing storage, but it's an option.
	 */
	void preDeserialize(ofXml& xml, bool clear = true)
	{
		if (clear) this->clear(false); // Don't notify, since we are going to deserialize soon

//		auto root = xml.findFirst("//" + parameterGroup.getEscapedName());
		auto path = "//" + parameterGroup.getEscapedName();


		auto search = xml.findFirst(path);

		if (!search)
		{
			ofLogNotice(__PRETTY_FUNCTION__) << "Could not find " << path;
			return;
		}
		auto children = search.getChildren();
		for (auto child : children)
		{
			ofLogVerbose(__PRETTY_FUNCTION__) << child.getName();
			if (child.getValue().size() == 0)
			{
				ofLogError(__PRETTY_FUNCTION__) << "Ignoring empty child in group " << parameterGroup.getName();
				continue;
			}
			addEntry();
		}
	}

	/**
	 * @brief Returns a copy of the parameter storage vector. I should make this a bit more
	 * const-y and safe-y.
	 * @return
	 */
	std::vector<std::shared_ptr<ofParameter<ParameterType>>> getParameters() const
	{
		return parameters;
	}

	/**
	 * @brief Returns the values of the ofParameter collection wrapped in shared_ptr's.
	 * @return
	 */
	std::vector<std::shared_ptr<ParameterType>> getCollection()
	{
		std::vector<std::shared_ptr<ParameterType>> collection;
		for (auto& param : parameters)
		{
			collection.emplace_back(std::make_shared<ofPath>(param->get()));
		}

		return collection;
	}

	size_t size()
	{
		assert(parameters.size() == parameterGroup.size()); // Sanity check
		return parameters.size();
	}

	/**
	 * @brief Notifies the listeners of the clollectionChangedEvent. You shouldn't have to call this yourself
	 * in most situations.
	 */
	void notify()
	{
		collectionChangedEvent.notify(*this);
	}
};


#endif //NERVOUSSTRUCTUREOF_ParameterCollection_HPP
