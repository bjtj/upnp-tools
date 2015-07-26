#ifndef __UPNP_SERVICE_HPP__
#define __UPNP_SERVICE_HPP__

#include <vector>

namespace UPNP {

	class UPnPServiceBuilder;

	/**
	 * @brief upnp state variable
	 */
	class UPnPStateVariable {
	private:
	public:
		
		UPnPStateVariable();
		virtual ~UPnPStateVariable();

		std::string getName();
		std::string getDataType();
		bool isSet(std::string flag);
		std::vector<std::string> getAllowedValueList();
	};

	/**
	 * @brief upnp action argument
	 */
	class UPnPActionArgument {
	private:
	public:
		
		UPnPActionArgument();
		virtual ~UPnPActionArgument();

		bool inDirection();
		bool outDirection();
		std::string getName();
		std::string getStateVariableName();
	};

	/**
	 * @brief upnp action
	 */
	class UPnPAction {
	private:
		
		std::vector<UPnPActionArgument> arguments;
		std::vector<UPnPStateVariable> stateVariables;
		
	public:
		
		UPnPAction();
		virtual ~UPnPAction();

		virtual std::string getName();

		std::vector<UPnPActionArgument> & getArguments();
		std::vector<UPnPStateVariable> & getStateVariables();
	};


	/**
	 * @brief upnp service
	 */
	class UPnPService {
	private:
		
		std::vector<UPnPAction> actions;
		
	public:
		
		UPnPService();
		virtual ~UPnPService();

		UPnPAction & getAction(std::string name);

		virtual int sendAction(UPnPActionRequest & request);
		virtual int subscribe(UPnPSubscriber * subscriber);
		virtual int unsubscribe(UPnPSubscriber * subscriber);

		UPnPServiceBuilder * getBuilder();
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

		void addAction(UPnpAction & action);
		void addStateVariable(UPnPStateVariable & stateVariable);
	};
	
}

#endif
