#include "utils.hpp"
#include <liboslayer/os.hpp>
#include <liboslayer/Uuid.hpp>
#include <libupnp-tools/UPnPDeviceBuilder.hpp>
#include <libupnp-tools/UPnPDeviceDeserializer.hpp>
#include <libupnp-tools/UPnPDeviceProfileBuilder.hpp>
#include <libupnp-tools/UPnPResourceManager.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace HTTP;
using namespace UPNP;

static string dd(const string & uuid);
static string scpd_cm();
static string scpd_cd();

static void test_device_profile_builder() {
	
	UuidGeneratorVersion1 gen;
	string uuid = gen.generate();

	UPnPResourceManager & resMan = UPnPResourceManager::instance();
	
	resMan.properties()["/device.xml"] = dd(uuid);
	resMan.properties()["/scpd/urn:schemas-upnp-org:service:ContentDirectory:1"] = scpd_cd();
	resMan.properties()["/scpd/urn:schemas-upnp-org:service:ConnectionManager:1"] = scpd_cm();

	UPnPDeviceBuilder builder(Url("prop:///device.xml"));
	AutoRef<UPnPDevice> device = builder.execute();
	device->setUdn(UDN("uuid:" + uuid));
	UPnPDeviceProfile profile(device);

	ASSERT(profile.udn().toString(), ==, ("uuid:" + uuid));
	ASSERT(profile.deviceTypes()[0], ==, "urn:schemas-upnp-org:device:MediaServer:1");
	ASSERT(profile.allServices()[0]->serviceType(), ==, "urn:schemas-upnp-org:service:ContentDirectory:1");
	ASSERT(profile.allServices()[1]->serviceType(), ==, "urn:schemas-upnp-org:service:ConnectionManager:1");

	device = UPnPDeviceDeserializer::deserializeDevice(profile.deviceDescription());
	ASSERT(device->udn().toString(), ==, ("uuid:" + uuid));
}

static void test_build_from_file() {
	
	UPnPDeviceBuilder builder(Url("file://" + File::merge(DATA_PATH, "/dms.xml")));
	AutoRef<UPnPDevice> device = builder.execute();
	UPnPDeviceProfile profile(device);

	vector< AutoRef<UPnPService> > services = profile.allServices();
	for (vector< AutoRef<UPnPService> >::iterator iter = services.begin();
		 iter != services.end(); iter++)
	{
		cout << (*iter)->scpdUrl() << endl;
		cout << (*iter)->controlUrl() << endl;
		cout << (*iter)->eventSubUrl() << endl;
	}
}

static void test_builder() {

	Url url("file://" + File::merge(DATA_PATH, "/dms.xml"));

	UPnPDeviceBuilder builder(url);
	UPnPDeviceProfile profile(builder.execute());

	vector< AutoRef<UPnPService> > profiles = profile.allServices();
	for (vector< AutoRef<UPnPService> >::iterator iter = profiles.begin();
		 iter != profiles.end(); iter++)
	{
		cout << (*iter)->scpdUrl() << endl;
		cout << (*iter)->controlUrl() << endl;
		cout << (*iter)->eventSubUrl() << endl;
	}
}

int main(int argc, char *args[]) {

	test_device_profile_builder();
	test_build_from_file();
	test_builder();
    
    return 0;
}


static string dd(const string & uuid) {
	return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
		"<root xmlns=\"urn:schemas-upnp-org:device-1-0\" xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">\r\n"
		"<specVersion>\r\n"
		"<major>1</major>\r\n"
		"<minor>0</minor>\r\n"
		"</specVersion>\r\n"
		"<device>\r\n"
		"<deviceType>urn:schemas-upnp-org:device:MediaServer:1</deviceType>\r\n"
		"<friendlyName>Dummy Device</friendlyName>\r\n"
		"<manufacturer>TJ Apps</manufacturer>\r\n"
		"<manufacturerURL>http://example.com/</manufacturerURL>\r\n"
		"<modelDescription>TJ Apps</modelDescription>\r\n"
		"<modelName>Dummy Device (DMS like...)</modelName>\r\n"
		"<modelURL>http://example.com/</modelURL>\r\n"
		"<modelNumber>0.1</modelNumber>\r\n"
		"<serialNumber/>\r\n"
		"<UDN>uuid:" + uuid + "</UDN>\r\n"
		"<dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">DMS-1.50</dlna:X_DLNADOC>\r\n"
		"<serviceList>\r\n"
		"<service>\r\n"
		"<serviceType>urn:schemas-upnp-org:service:ContentDirectory:1</serviceType>\r\n"
		"<serviceId>urn:upnp-org:serviceId:ContentDirectory</serviceId>\r\n"
		"<SCPDURL>/scpd/urn:schemas-upnp-org:service:ContentDirectory:1</SCPDURL>\r\n"
		"<controlURL>/control/urn:schemas-upnp-org:service:ContentDirectory:1</controlURL>\r\n"
		"<eventSubURL>/event/urn:schemas-upnp-org:service:ContentDirectory:1</eventSubURL>\r\n"
		"</service>\r\n"
		"<service>\r\n"
		"<serviceType>urn:schemas-upnp-org:service:ConnectionManager:1</serviceType>\r\n"
		"<serviceId>urn:upnp-org:serviceId:ConnectionManager</serviceId>\r\n"
		"<SCPDURL>/scpd/urn:schemas-upnp-org:service:ConnectionManager:1</SCPDURL>\r\n"
		"<controlURL>/control/urn:schemas-upnp-org:service:ConnectionManager:1</controlURL>\r\n"
		"<eventSubURL>/event/urn:schemas-upnp-org:service:ConnectionManager:1</eventSubURL>\r\n"
		"</service>\r\n"
		"</serviceList>\r\n"
		"</device>\r\n"
		"</root>\r\n";
}

static string scpd_cm() {
	return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">"
		"<specVersion>"
		"<major>1</major>"
		"<minor>0</minor>"
		"</specVersion>"
		"<actionList>"
		"<action>"
		"<name>GetCurrentConnectionInfo</name>"
		"<argumentList>"
		"<argument>"
		"<name>ConnectionID</name>"
		"<direction>in</direction>"
		"<relatedStateVariable>A_ARG_TYPE_ConnectionID</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>RcsID</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>A_ARG_TYPE_RcsID</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>AVTransportID</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>A_ARG_TYPE_AVTransportID</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>ProtocolInfo</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>A_ARG_TYPE_ProtocolInfo</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>PeerConnectionManager</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>A_ARG_TYPE_ConnectionManager</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>PeerConnectionID</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>A_ARG_TYPE_ConnectionID</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>Direction</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>A_ARG_TYPE_Direction</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>Status</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>A_ARG_TYPE_ConnectionStatus</relatedStateVariable>"
		"</argument>"
		"</argumentList>"
		"</action>"
		"<action>"
		"<name>GetProtocolInfo</name>"
		"<argumentList>"
		"<argument>"
		"<name>Source</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>SourceProtocolInfo</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>Sink</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>SinkProtocolInfo</relatedStateVariable>"
		"</argument>"
		"</argumentList>"
		"</action>"
		"<action>"
		"<name>GetCurrentConnectionIDs</name>"
		"<argumentList>"
		"<argument>"
		"<name>ConnectionIDs</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>CurrentConnectionIDs</relatedStateVariable>"
		"</argument>"
		"</argumentList>"
		"</action>"
		"</actionList>"
		"<serviceStateTable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_ProtocolInfo</name>"
		"<dataType>string</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_ConnectionStatus</name>"
		"<dataType>string</dataType>"
		"<allowedValueList>"
		"<allowedValue>OK</allowedValue>"
		"<allowedValue>ContentFormatMismatch</allowedValue>"
		"<allowedValue>InsufficientBandwidth</allowedValue>"
		"<allowedValue>UnreliableChannel</allowedValue>"
		"<allowedValue>Unknown</allowedValue>"
		"</allowedValueList>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_AVTransportID</name>"
		"<dataType>i4</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_RcsID</name>"
		"<dataType>i4</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_ConnectionID</name>"
		"<dataType>i4</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_ConnectionManager</name>"
		"<dataType>string</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"yes\">"
		"<name>SourceProtocolInfo</name>"
		"<dataType>string</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"yes\">"
		"<name>SinkProtocolInfo</name>"
		"<dataType>string</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_Direction</name>"
		"<dataType>string</dataType>"
		"<allowedValueList>"
		"<allowedValue>Input</allowedValue>"
		"<allowedValue>Output</allowedValue>"
		"</allowedValueList>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"yes\">"
		"<name>CurrentConnectionIDs</name>"
		"<dataType>string</dataType>"
		"</stateVariable>"
		"</serviceStateTable>"
		"</scpd>";
}

static string scpd_cd() {
	return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">"
		"<specVersion>"
		"<major>1</major>"
		"<minor>0</minor>"
		"</specVersion>"
		"<actionList>"
		"<action>"
		"<name>Browse</name>"
		"<argumentList>"
		"<argument>"
		"<name>ObjectID</name>"
		"<direction>in</direction>"
		"<relatedStateVariable>A_ARG_TYPE_ObjectID</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>BrowseFlag</name>"
		"<direction>in</direction>"
		"<relatedStateVariable>A_ARG_TYPE_BrowseFlag</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>Filter</name>"
		"<direction>in</direction>"
		"<relatedStateVariable>A_ARG_TYPE_Filter</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>StartingIndex</name>"
		"<direction>in</direction>"
		"<relatedStateVariable>A_ARG_TYPE_Index</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>RequestedCount</name>"
		"<direction>in</direction>"
		"<relatedStateVariable>A_ARG_TYPE_Count</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>SortCriteria</name>"
		"<direction>in</direction>"
		"<relatedStateVariable>A_ARG_TYPE_SortCriteria</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>Result</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>A_ARG_TYPE_Result</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>NumberReturned</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>A_ARG_TYPE_Count</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>TotalMatches</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>A_ARG_TYPE_Count</relatedStateVariable>"
		"</argument>"
		"<argument>"
		"<name>UpdateID</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>A_ARG_TYPE_UpdateID</relatedStateVariable>"
		"</argument>"
		"</argumentList>"
		"</action>"
		"<action>"
		"<name>GetSortCapabilities</name>"
		"<argumentList>"
		"<argument>"
		"<name>SortCaps</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>SortCapabilities</relatedStateVariable>"
		"</argument>"
		"</argumentList>"
		"</action>"
		"<action>"
		"<name>GetSystemUpdateID</name>"
		"<argumentList>"
		"<argument>"
		"<name>Id</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>SystemUpdateID</relatedStateVariable>"
		"</argument>"
		"</argumentList>"
		"</action>"
		"<action>"
		"<name>GetSearchCapabilities</name>"
		"<argumentList>"
		"<argument>"
		"<name>SearchCaps</name>"
		"<direction>out</direction>"
		"<relatedStateVariable>SearchCapabilities</relatedStateVariable>"
		"</argument>"
		"</argumentList>"
		"</action>"
		"</actionList>"
		"<serviceStateTable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_BrowseFlag</name>"
		"<dataType>string</dataType>"
		"<allowedValueList>"
		"<allowedValue>BrowseMetadata</allowedValue>"
		"<allowedValue>BrowseDirectChildren</allowedValue>"
		"</allowedValueList>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"yes\">"
		"<name>ContainerUpdateIDs</name>"
		"<dataType>string</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"yes\">"
		"<name>SystemUpdateID</name>"
		"<dataType>ui4</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_Count</name>"
		"<dataType>ui4</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_SortCriteria</name>"
		"<dataType>string</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>SortCapabilities</name>"
		"<dataType>string</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_Index</name>"
		"<dataType>ui4</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_ObjectID</name>"
		"<dataType>string</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_UpdateID</name>"
		"<dataType>ui4</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_Result</name>"
		"<dataType>string</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>SearchCapabilities</name>"
		"<dataType>string</dataType>"
		"</stateVariable>"
		"<stateVariable sendEvents=\"no\">"
		"<name>A_ARG_TYPE_Filter</name>"
		"<dataType>string</dataType>"
		"</stateVariable>"
		"</serviceStateTable>"
		"</scpd>";
}
