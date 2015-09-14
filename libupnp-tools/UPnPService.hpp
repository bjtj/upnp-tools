#ifndef __UPNP_SERVICE_HPP__
#define __UPNP_SERVICE_HPP__

#include <vector>

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

	public:
		static UPnPService EMPTY;
		
	public:
		UPnPService();
		UPnPService(std::string serviceType);
		virtual ~UPnPService();

		std::string getServiceType();
		UPnPAction getAction(std::string name);

		// virtual int sendAction(UPnPActionRequest & request);
		// virtual int subscribe(UPnPSubscriber * subscriber);
		// virtual int unsubscribe(UPnPSubscriber * subscriber);

		bool isEmpty();

		bool operator==(const UPnPService &other) const;
	};

	/**
	 * @brief upnp service builder
	 */
	class UPnPServiceBuilder {
	private:
		UPnPService * service;
		
	public:
		UPnPServiceBuilder();
		virtual ~UPnPServiceBuilder();

		void addAction(UPnPAction & action);
		void addStateVariable(UPnPStateVariable & stateVariable);
	};
	
}

#endif
