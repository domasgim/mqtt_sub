module("luci.controller.mqtt_sub_controller", package.seeall)

function index()
	-- entry({"admin", "services", "mqtt_sub_model"}, cbi("mqtt_sub_model"), _("MQTT subscriber"),105)
	entry( { "admin", "services", "mqtt", "subscriber" }, cbi("mqtt_sub_model"), _("Subscriber"), 3).leaf = true
end