require("luci.config")

map = Map("mqtt_sub")

s = map:section(NamedSection, "config", "mqtt_sub", "MQTT subscriber")

flag = s:option(Flag, "enable", "Enable", "Select to subscribe to a topic")
flag.default = "0"

host = s:option(Value, "host", "Host", translate("MQTT broker hostname or IP"))
host.datatype = "host"
host.default = "127.0.0.1"

port = s:option(Value, "port", "Local Port", translate("Specify local port which the MQTT subscriber will be listen to"))
port.datatype = "port"
port.default = "1883"

return map