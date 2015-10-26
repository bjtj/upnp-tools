#ifndef __UPNP_SERVICE_HPP__
#define __UPNP_SERVICE_HPP__

#include <map>
#include <vector>
#include <string>

#include "UPnPStateVariable.hpp"
#include "UPnPAction.hpp"

namespace UPNP {

	/**
	 * @brief upnp service
	 */
	class UPnPService {
	private:
		std::string serviceType;
		std::vector<UPnPAction> actions;
		std::vector<UPnPStateVariable> stateVariables;
		std::map<std::string, std::string> properties;

	public:
		static UPnPService EMPTY;
		
	public:
		UPnPService();
		virtual ~UPnPService();

		std::string getServiceType();
		UPnPAction getAction(std::string name);
		std::vector<UPnPAction> & getActions();
		std::vector<UPnPStateVariable> & getStateVariables();

		// virtual int sendAction(UPnPActionRequest & request);
		// virtual int subscribe(UPnPSubscriber * subscriber);
		// virtual int unsubscribe(UPnPSubscriber * subscriber);

		bool isEmpty();

		void setServiceType(const std::string & serviceType);
		void setActions(std::vector<UPnPAction> & actions);
		void setStateVariables(std::vector<UPnPStateVariable> & stateVariables);

		bool operator==(const UPnPService &other) const;
		std::string & operator[](const std::string & name);
	};
}

#endif
