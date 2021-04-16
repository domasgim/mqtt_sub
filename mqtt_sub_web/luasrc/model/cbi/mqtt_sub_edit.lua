local m, s, o
local sid = arg[1] and arg[1] or ""
local topic
local topic_name
local qos

m = Map("mqtt_sub")
m.redirect = luci.dispatcher.build_url("admin/services/mqtt/subscriber/topics/")

local topic_name = m.uci:get(sid, "topic") or ""
local qos = m.uci:get(sid, "qos") or ""

s = m:section(NamedSection, sid, "topic", translate("Topic settings"))

topic = s:option(Value, "topic", translate("Topic name"))
topic.datatype = "string"
topic.maxlength = 65536
topic.placeholder = translate("Topic")
topic.rmempty = false
topic.parse = function(self, section, novld, ...)
	local value = self:formvalue(section)
	if value == nil or value == "" then
		self.map:error_msg(translate("Topic name can not be empty"))
		self.map.save = false
	end
	Value.parse(self, section, novld, ...)
end

qos = s:option(ListValue, "qos", translate("QoS level"), translate("The publish/subscribe QoS level used for this topic"))
qos:value("0", "At most once (0)")
qos:value("1", "At least once (1)")
qos:value("2", "Exactly once (2)")
qos.rmempty=false
qos.default="0"

return m