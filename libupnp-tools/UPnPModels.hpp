#ifndef __UPNP_MODELS_HPP__
#define __UPNP_MODELS_HPP__

#include <string>
#include <map>
#include <vector>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/StringElement.hpp>
#include <liboslayer/PropertyMap.hpp>
#include <libhttp-server/Url.hpp>

namespace UPNP {

	class UPnPService;
	class UPnPDevice;

	/**
	 *
	 */
	class UPnPModelObject {
	private:
		// UTIL::LinkedStringMap props;
		UTIL::PropertyMap props;
	public:
		UPnPModelObject() {}
		virtual ~UPnPModelObject() {}

		UTIL::PropertyMap & getProperties() {return props;}

		std::string & operator[] (const std::string & name) {
			return props[name];
		}
	};

	/**
	 *
	 */
	class UPnPStateVariable : public UPnPModelObject {
	private:
		std::string _name;
		std::string _dataType;
		std::string _defValue;
		std::vector<std::string> _allowedValueList;
	public:
		UPnPStateVariable() {}
		virtual ~UPnPStateVariable() {}

		std::string & name() {return _name;}
		std::string & dataType() {return _dataType;}
		std::string & defaultValue() {return _defValue;}
		std::vector<std::string> & allowedValueList() {return _allowedValueList;}
		void addAllowedValue(const std::string & allowedValue) {_allowedValueList.push_back(allowedValue);}
		bool hasAllowedValues() {return _allowedValueList.size() > 0;}
	};

	/**
	 *
	 */
	class UPnPArgument : public UPnPModelObject {
	public:
		static const int UNKNOWN_DIRECTION = 0;
		static const int IN_DIRECTION = 1;
		static const int OUT_DIRECTION = 2;
	private:
		std::string _name;
		int _direction;
		std::string _stateVariableName;
	public:
		UPnPArgument() : _direction(UNKNOWN_DIRECTION) {}
		virtual ~UPnPArgument() {}
		bool in() {return _direction == IN_DIRECTION;}
		bool out() {return _direction == OUT_DIRECTION;}
		std::string & name() {return _name;}
		int & direction() {return _direction;}
		std::string & stateVariableName() {return _stateVariableName;}
	};

	/**
	 *
	 */
	class UPnPAction : public UPnPModelObject {
	private:
		std::string _name;
		std::vector<UPnPArgument> _arguments;
	
	public:
		UPnPAction() {}
		virtual ~UPnPAction() {}

		std::string & name() {
			return _name;
		}
		std::vector<UPnPArgument> & arguments() {
			return _arguments;
		}
		void addArgument(UPnPArgument arg) {
			_arguments.push_back(arg);
		}
	};

	/**
	 *
	 */
	class UPnPService : public UPnPModelObject{
	private:
		UPnPDevice * device;
		std::vector<UPnPAction> _actions;
		std::vector<UPnPStateVariable> _stateVariables;
	public:
		UPnPService();
		UPnPService(UPnPDevice * device);
		virtual ~UPnPService();
		std::string getServiceType();
		std::string getServiceId();
		std::string getScpdUrl();
		std::string getControlUrl();
		std::string getEventSubUrl();
		void setDevice(UPnPDevice * device);
		UPnPDevice * getDevice();
		HTTP::Url makeScpdUrl();
		std::vector<UPnPAction> & actions();
		UPnPAction getAction(const std::string & actionName);
		void addAction(UPnPAction action);
		std::vector<UPnPStateVariable> & stateVariables();
		UPnPStateVariable getStateVariable(const std::string & stateVariableName);
		void addStateVariable(UPnPStateVariable stateVariable);
	};

	/**
	 *
	 */
	class UPnPDevice : public UPnPModelObject {
	private:
		UPnPDevice * parent;
		std::vector<UTIL::AutoRef<UPnPDevice> > _devices;
		std::vector<UTIL::AutoRef<UPnPService> > _services;
		HTTP::Url _baseUrl;
		
	public:
		UPnPDevice();
		UPnPDevice(UPnPDevice * parent);
		virtual ~UPnPDevice();
		void setParent(UPnPDevice * parent);
		UTIL::AutoRef<UPnPDevice> prepareDevice();
		void addDevice(UTIL::AutoRef<UPnPDevice> device);
		void addService(UTIL::AutoRef<UPnPService> service);
		bool hasService(const std::string & serviceType);
		UTIL::AutoRef<UPnPService> getService(const std::string & serviceType);
		std::vector<UTIL::AutoRef<UPnPDevice> > & devices();
		std::vector<UTIL::AutoRef<UPnPService> > & services();
		std::string getUdn();	
		std::string getFriendlyName();
		std::string getDeviceType();
		HTTP::Url & baseUrl();
	};	
	
}

#endif
