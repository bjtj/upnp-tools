#ifndef __UPNP_SERVICE_HPP__
#define __UPNP_SERVICE_HPP__

#include <map>
#include <vector>
#include <string>

#include <liboslayer/StringElement.hpp>

#include "UPnPStateVariable.hpp"
#include "UPnPAction.hpp"

namespace UPNP {

	class Scpd {
	private:
		std::vector<UPnPAction> actions;
		std::vector<UPnPStateVariable> stateVariables;
		std::map<std::string, std::string> properties;

	public:
		Scpd();
		virtual ~Scpd();

		void clear();

		UPnPAction getAction(std::string name);
		std::vector<UPnPAction> & getActions();
        const std::vector<UPnPAction> & getActions() const;
		std::vector<UPnPStateVariable> & getStateVariables();
        const std::vector<UPnPStateVariable> & getStateVariables() const;

		void setActions(std::vector<UPnPAction> & actions);
		void setStateVariables(std::vector<UPnPStateVariable> & stateVariables);

		std::string & operator[](const std::string & name);
	};

	/**
	 * @brief upnp service
	 */
	class UPnPService {
	private:
        UTIL::LinkedStringMap properties;
		bool scpdBind;
		Scpd scpd;
        std::string baseUrl;

	public:
		static UPnPService EMPTY;
		
	public:
		UPnPService();
		virtual ~UPnPService();

		bool empty();
        void clear();
		void setServiceType(const std::string & serviceType);
        std::string getServiceType() const;
		void setScpd(const Scpd & scpd);
		Scpd & getScpd();
		const Scpd & getScpd() const;
        void setBaseUrl(const std::string & baseUrl);
        std::string getBaseUrl() const;

		bool isScpdBind() const;
        
		std::string getProperty(const std::string & name) const;
        UTIL::LinkedStringMap & getProperties();
        const UTIL::LinkedStringMap & getProperties() const;

        std::string & operator[](const std::string & name);
	};
}

#endif
