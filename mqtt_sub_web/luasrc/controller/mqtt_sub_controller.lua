module("luci.controller.mqtt_sub_controller", package.seeall)

function index()
    entry( { "admin", "services", "mqtt", "subscriber" },firstchild(), _("Subscriber"), 7)
    entry( { "admin", "services", "mqtt", "subscriber", "general" }, cbi("mqtt_sub_general"), _("General"), 8).leaf = true
    entry( { "admin", "services", "mqtt", "subscriber", "topics" }, 
        arcombine(cbi("mqtt_sub_topics"), cbi("mqtt_sub_edit")), _("Topics"), 11).leaf = true
    entry( { "admin", "services", "mqtt", "subscriber", "logs" }, cbi("mqtt_sub_logs"), _("Logs"), 12).leaf = true
end