#ifndef __UPNP_SERVICE_HPP__
#define __UPNP_SERVICE_HPP__

#include <map>
#include <vector>
#include <string>

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

		UPnPAction getAction(std::string name);
		std::vector<UPnPAction> & getActions();
		std::vector<UPnPStateVariable> & getStateVariables();

		void setActions(std::vector<UPnPAction> & actions);
		void setStateVariables(std::vector<UPnPStateVariable> & stateVariables);

		std::string & operator[](const std::string & name);
	};

	/**
	 * @brief upnp service
	 */
	class UPnPService {
	private:
		mutable std::map<std::string, std::string> properties;
		bool scpdBind;
		Scpd scpd;
        std::string baseUrl;

	public:
		static UPnPService EMPTY;
		
	public:
		UPnPService();
		virtual ~UPnPService();

		bool empty();
		void setServiceType(const std::string & serviceType);
        std::string getServiceType() const;
		void setScpd(const Scpd & scpd);
		Scpd & getScpd();
        void setBaseUrl(const std::string & baseUrl);
        std::string getBaseUrl() const;

		bool isScpdBind() const;

		std::string & operator[](const std::string & name) const;
	};
}

#endif
