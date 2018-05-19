#ifndef __UPNP_MODELS_HPP__
#define __UPNP_MODELS_HPP__

#include <string>
#include <map>
#include <vector>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/StringElements.hpp>
#include <liboslayer/PropertyMap.hpp>
#include <libhttp-server/Url.hpp>

namespace upnp {

	class UPnPService;
	class UPnPDevice;

	/**
	 * max age
	 */
	class MaxAge {
	private:
		unsigned long _second;
	public:
		MaxAge(unsigned long second);
		virtual ~MaxAge();
		unsigned long & second();
		static MaxAge fromString(const std::string & phrase);
		std::string toString() const;
		static std::string toString(unsigned long second);
	};

	/**
	 * callback urls
	 */
	class CallbackUrls {
	private:
		std::vector<std::string> _urls;
	public:
		CallbackUrls(const std::vector<std::string> & urls);
		virtual ~CallbackUrls();
		std::vector<std::string> & urls();
		static CallbackUrls fromString(const std::string & phrase);
		std::string toString() const;
		static std::string toString(const std::vector<std::string> & _urls);
	};

	/**
	 * second
	 */
	class Second {
	private:
		unsigned long _second;
	public:
		Second(unsigned long second);
		virtual ~Second();
		unsigned long & second();
		unsigned long second() const;
		unsigned long milli() const;
		static Second fromString(const std::string & phrase);
		std::string toString() const;
		static std::string toString(unsigned long second);
	};

	

	/**
	 * @brief upnp base object
	 */
	class UPnPModelObject {
	private:
		osl::PropertyMap _props;
		osl::LinkedStringMap _meta;
	public:
		UPnPModelObject() {}
		virtual ~UPnPModelObject() {}
		osl::PropertyMap & properties() {
			return _props;
		}
		osl::PropertyMap properties() const {
			return _props;
		}
		osl::LinkedStringMap & meta() {
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
		std::vector< osl::AutoRef<UPnPDevice> > _childDevices;
		std::vector< osl::AutoRef<UPnPService> > _services;
		http::Url _baseUrl;
		
	public:
		UPnPDevice();
		UPnPDevice(UPnPDevice * parent);
		virtual ~UPnPDevice();
		void setParent(UPnPDevice * parent);
		osl::AutoRef<UPnPDevice> prepareDevice();
		void addDevice(osl::AutoRef<UPnPDevice> device);
		void addService(osl::AutoRef<UPnPService> service);
		osl::AutoRef<UPnPService> getService(const std::string & serviceType);
		osl::AutoRef<UPnPService> getServiceWithScpdUrl(const std::string & scpdUrl);
		osl::AutoRef<UPnPService> getServiceWithControlUrl(const std::string & controlUrl);
		osl::AutoRef<UPnPService> getServiceWithEventSubUrl(const std::string & eventSubUrl);
		std::vector<osl::AutoRef<UPnPDevice> > & childDevices();
		std::vector<osl::AutoRef<UPnPService> > & services();
		std::vector< osl::AutoRef<UPnPDevice> > allChildDevices();
		std::vector< osl::AutoRef<UPnPService> > allServices();
		std::string & udn();
		void setUdn(const std::string & udn);
		const std::string & udn() const;
		std::string & friendlyName();
		const std::string & friendlyName() const;
		std::string & deviceType();
		const std::string & deviceType() const;
		http::Url & baseUrl();
		std::string formatUrl(const std::string & url, osl::AutoRef<UPnPService> service);
		void setScpdUrl(const std::string & scpdUrl);
		void setControlUrl(const std::string & controlUrl);
		void setEventSubUrl(const std::string & eventSubUrl);
		
	};	
}

#endif
