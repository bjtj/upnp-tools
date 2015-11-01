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
		std::map<std::string, std::string> properties;

		Scpd scpd;

	public:
		static UPnPService EMPTY;
		
	public:
		UPnPService();
		virtual ~UPnPService();

		std::string getServiceType();
		bool empty();
		void setServiceType(const std::string & serviceType);

		void setScpd(const Scpd & scpd);
		Scpd & getScpd();

		std::string & operator[](const std::string & name);
	};
}

#endif
