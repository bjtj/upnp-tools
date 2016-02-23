#ifndef __UPNP_MODELS_HPP__
#define __UPNP_MODELS_HPP__

#include <string>
#include <map>
#include <vector>
#include <liboslayer/AutoRef.hpp>
#include <libhttp-server/Url.hpp>

namespace UPNP {

	class UPnPService;
	class UPnPDevice;

	/**
	 *
	 */
	class UPnPObject {
	private:
		std::map<std::string, std::string> props;
	public:
		UPnPObject() {}
		virtual ~UPnPObject() {}

		std::map<std::string, std::string> & getProperties() {return props;}

		std::string & operator[] (const std::string & name) {
			return props[name];
		}
	};

	/**
	 *
	 */
	class UPnPStateVariable : public UPnPObject {
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
	};

	/**
	 *
	 */
	class UPnPArgument : public UPnPObject {
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
		bool out() {return _direction = OUT_DIRECTION;}
		std::string & name() {return _name;}
		int & direction() {return _direction;}
		std::string & stateVariableName() {return _stateVariableName;}
	};

	/**
	 *
	 */
	class UPnPAction : public UPnPObject {
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
	class UPnPService : public UPnPObject{
	private:
		UPnPDevice * device;
		std::vector<UPnPAction> _actions;
		std::vector<UPnPStateVariable> _stateVariables;
	public:
		UPnPService() : device(NULL) {}
		UPnPService(UPnPDevice * device) : device(device) {}
		virtual ~UPnPService() {}
		std::string getServiceType() {
			return getProperties()["serviceType"];
		}
		std::string getScpdUrl() {
			return getProperties()["SCPDURL"];
		}
		std::string getControlUrl() {
			return getProperties()["controlURL"];
		}
		std::string getEventSubUrl() {
			return getProperties()["eventSubURL"];
		}
		void setDevice(UPnPDevice * device) {
			this->device = device;
		}
		UPnPDevice * getDevice() {
			return device;
		}
		std::vector<UPnPAction> & actions() {return _actions;}
		UPnPAction getAction(const std::string & actionName) {
			for (std::vector<UPnPAction>::iterator iter = _actions.begin(); iter != _actions.end(); iter++) {
				if (iter->name() == actionName) {
					return *iter;
				}
			}
			return UPnPAction();
		}
		void addAction(UPnPAction action) {_actions.push_back(action);}
		std::vector<UPnPStateVariable> & stateVariables() {return _stateVariables;}
	};

	/**
	 *
	 */
	class UPnPDevice : public UPnPObject {
	private:
		UPnPDevice * parent;
		std::vector<UTIL::AutoRef<UPnPDevice> > _devices;
		std::vector<UTIL::AutoRef<UPnPService> > _services;
		HTTP::Url _baseUrl;
		
	public:
		UPnPDevice() : parent(NULL) {}
		UPnPDevice(UPnPDevice * parent) : parent(parent) {}
		virtual ~UPnPDevice() {}

		void setParent(UPnPDevice * parent) {
			this->parent = parent;
		}

		UTIL::AutoRef<UPnPDevice> prepareDevice() {
			UTIL::AutoRef<UPnPDevice> device(new UPnPDevice(this));
			device->baseUrl() = _baseUrl;
			addDevice(device);
			return device;
		}

		void addDevice(UTIL::AutoRef<UPnPDevice> device) {
			device->setParent(this);
			_devices.push_back(device);
		}

		void addService(UTIL::AutoRef<UPnPService> service) {
			service->setDevice(this);
			_services.push_back(service);
		}

		UTIL::AutoRef<UPnPService> getService(const std::string & serviceType) {
			for (std::vector<UTIL::AutoRef<UPnPService> >::iterator iter = _services.begin(); iter != _services.end(); iter++) {
				if ((*iter)->getServiceType() == serviceType) {
					return *iter;
				}
			}
			return NULL;
		}

		std::vector<UTIL::AutoRef<UPnPDevice> > & devices() {
			return _devices;
		}

		std::vector<UTIL::AutoRef<UPnPService> > & services() {
			return _services;
		}

		std::string getUdn() {
			return getProperties()["UDN"];
		}
	
		std::string getFriendlyName() {
			return getProperties()["friendlyName"];
		}

		std::string getDeviceType() {
			return getProperties()["deviceType"];
		}

		HTTP::Url & baseUrl() {return _baseUrl;}
	};	
	
}

#endif
