local m, s, o
local sid = arg[1] and arg[1] or ""
local topic
local topic_name
local qos

m = Map("mqtt_sub")
m.redirect = luci.dispatcher.build_url("admin/services/mqtt/subscriber/topics/")
-- if not sid or not m.uci:get(m.config, arg[1]) then
--     luci.http.redirect(m.redirect)
-- end

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

s = m:section(TypedSection, sid, "topic", translate("Events configuration"), translate("Events configuration"))
s.template = "cbi/tblsection"
s.template_addremove = "mqtt_sub/event_details"
s.addremove = true
s.anonymous = true
s.add_toggled = true
s.novaluetext = translate("No requests configured for this slave device yet")

function s.create(self, section)
	local created = TypedSection.create(self)
	if created then
		m.uci:commit(m.config)
	end
end

json_val = s:option(Value, "json_val", translate("JSON value name"), translate("JSON value name to track when receiving a message"))

val_type = s:option(ListValue, "val_type", translate("Value type"), translate("Select value type of specified value name"))
val_type:value("string", translate("String"))
val_type:value("int", translate("Int"))

operator = s:option(ListValue, "operator", translate("Operator"), translate("Select operator to use when comparing the specified value"))
operator:value("=", translate("="))
operator:value("!=", translate("!="))
operator:value("<", translate("<"))
operator:value("<=", translate("<="))
operator:value(">", translate(">"))
operator:value(">=", translate(">="))

comparison_val = s:option(Value, "comparison_val", translate("Comparison"), translate("Enter the value that will be compared to"))

local is_group = false
mailGroup = s:option(ListValue, "email_group", translate("Email account"), translate("Recipient's email configuration <br/>(<a href=\"/cgi-bin/luci/admin/system/admin/group/email\" class=\"link\">configure it here</a>)"))
mailGroup:depends("action", "sendEmail")
m.uci:foreach("user_groups", "email", function(s)
	if s.senderemail then
		mailGroup:value(s.name, s.name)
		is_group = true
	end
end)
if not is_group then
	mailGroup:value(0, translate("No email accounts created"))
end

function mailGroup.parse(self, section, novld, ...)
	local val = self:formvalue(section)
	if val and val == "0" then
		self:add_error(section, "invalid", translate("No email accounts selected"))
	end
	Value.parse(self, section, novld, ...)
end

recpEmail = s:option(Value, "recip_email", translate("Recipient's email address"), translate("For whom you want to send an email to. Allowed characters (a-zA-Z0-9._%+@-)"))
recpEmail.datatype = "email"
recpEmail.placeholder = "mail@domain.com"

return m