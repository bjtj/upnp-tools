#ifndef __UPNP_MODELS_HPP__
#define __UPNP_MODELS_HPP__

#include <string>
#include <map>
#include <vector>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/StringElements.hpp>
#include <liboslayer/PropertyMap.hpp>
#include <libhttp-server/Url.hpp>
#include "UPnPTerms.hpp"

namespace UPNP {

	class UPnPService;
	class UPnPDevice;

	/**
	 * @brief upnp base object
	 */
	class UPnPModelObject {
	private:
		UTIL::PropertyMap _props;
		UTIL::LinkedStringMap _meta;
	public:
		UPnPModelObject() {}
		virtual ~UPnPModelObject() {}
		UTIL::PropertyMap & properties() {
			return _props;
		}
		UTIL::PropertyMap properties() const {
			return _props;
		}
		UTIL::LinkedStringMap & meta() {
			return _meta;
		}
		std::string & operator[] (const std::string & name) {
			return _props[name];
		}
	};

	/**
	 * @brief upnp state variable
	 */
	class UPnPStateVariable : public UPnPModelObject {
	private:
		bool _sendEvents;
		bool _multicast;
		std::string _name;
		std::string _dataType;
		std::string _defValue;
		std::vector<std::string> _allowedValueList;
		// TODO: implement allowedValueRange
		std::string _minimum;
		std::string _maximum;
		std::string _step;
	public:
		UPnPStateVariable() {}
		UPnPStateVariable(const std::string & name) : _name(name) {}
		virtual ~UPnPStateVariable() {}
		bool & sendEvents() {return _sendEvents;}
		bool & multicast() {return _multicast;}
		std::string & name() {return _name;}
		std::string & dataType() {return _dataType;}
		std::string & defaultValue() {return _defValue;}
		std::vector<std::string> & allowedValueList() {return _allowedValueList;}
		void addAllowedValue(const std::string & allowedValue) {_allowedValueList.push_back(allowedValue);}
		bool hasAllowedValues() {return _allowedValueList.size() > 0;}
		std::string & minimum() {return _minimum;}
		std::string & maximum() {return _maximum;}
		std::string & step() {return _step;}
	};

	/**
	 * @brief upnp argument
	 */
	class UPnPArgument : public UPnPModelObject {
	public:
		static const int UNKNOWN_DIRECTION = 0;
		static const int IN_DIRECTION = 1;
		static const int OUT_DIRECTION = 2;
	private:
		std::string _name;
		int _direction;
		std::string _relatedStateVariable;
	public:
		UPnPArgument() : _direction(UNKNOWN_DIRECTION) {}
		virtual ~UPnPArgument() {}
		bool in() {return _direction == IN_DIRECTION;}
		bool out() {return _direction == OUT_DIRECTION;}
		std::string & name() {return _name;}
		int & direction() {return _direction;}
		std::string & relatedStateVariable() {return _relatedStateVariable;}
	};

	/**
	 * @brief upnp action
	 */
	class UPnPAction : public UPnPModelObject {
	private:
		std::string _name;
		std::vector<UPnPArgument> _arguments;
	
	public:
		UPnPAction() {}
		UPnPAction(const std::string & name) : _name(name) {}
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
	 * @brief upnp scpd
	 */
	class UPnPScpd : public UPnPModelObject {
	private:
		std::vector<UPnPAction> _actions;
		std::vector<UPnPStateVariable> _stateVariables;
	public:
		UPnPScpd();
		virtual ~UPnPScpd();
		std::vector<UPnPAction> & actions();
		UPnPAction & action(const std::string & name);
		bool hasAction(const std::string & name);
		std::vector<UPnPStateVariable> & stateVariables();
		UPnPStateVariable & stateVariable(const std::string & name);
		bool hasStateVariable(const std::string & name);
	};

	/**
	 * @brief upnp service
	 */
	class UPnPService : public UPnPModelObject {
	private:
		// UPnPDevice * device;
		UPnPScpd _scpd;
	public:
		UPnPService();
		virtual ~UPnPService();
		std::string & serviceType();
		std::string & serviceId();
		std::string & scpdUrl();
		std::string & controlUrl();
		std::string & eventSubUrl();
		UPnPScpd & scpd();
	};

	/**
	 * @brief upnp device
	 */
	class UPnPDevice : public UPnPModelObject {
	private:
		UPnPDevice * parent;
		std::vector< OS::AutoRef<UPnPDevice> > _childDevices;
		std::vector<OS::AutoRef<UPnPService> > _services;
		HTTP::Url _baseUrl;
		
	public:
		UPnPDevice();
		UPnPDevice(UPnPDevice * parent);
		virtual ~UPnPDevice();
		void setParent(UPnPDevice * parent);
		OS::AutoRef<UPnPDevice> prepareDevice();
		void addDevice(OS::AutoRef<UPnPDevice> device);
		void addService(OS::AutoRef<UPnPService> service);
		OS::AutoRef<UPnPService> getService(const std::string & serviceType);
		OS::AutoRef<UPnPService> getServiceWithScpdUrl(const std::string & scpdUrl);
		OS::AutoRef<UPnPService> getServiceWithControlUrl(const std::string & controlUrl);
		OS::AutoRef<UPnPService> getServiceWithEventSubUrl(const std::string & eventSubUrl);
		std::vector<OS::AutoRef<UPnPDevice> > & childDevices();
		std::vector<OS::AutoRef<UPnPService> > & services();
		std::vector< OS::AutoRef<UPnPDevice> > allChildDevices();
		std::vector< OS::AutoRef<UPnPService> > allServices();
		UDN udn() const;
		void setUdn(const UDN & udn);
		std::string & friendlyName();
		std::string & deviceType();
		HTTP::Url & baseUrl();
		std::string formatUrl(const std::string & url, OS::AutoRef<UPnPService> service);
		void setScpdUrl(const std::string & scpdUrl);
		void setControlUrl(const std::string & controlUrl);
		void setEventSubUrl(const std::string & eventSubUrl);
		
	};	
}

#endif
