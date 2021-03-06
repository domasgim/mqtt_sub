
require("luci.config")

local uci = require("luci.model.uci").cursor()
local ds = require "luci.dispatcher"
local m ,s, o
local types = require "luci.cbi.datatypes"

m = Map("mqtt_sub")

s = m:section(TypedSection, "topic", translate("Topic list"), translate("This section displays MQTT topic instances") )
s.add_title = translate("Add New Topic")
s.addremove = true
s.noname = true
s.delete_alert = true
s.alert_message = translate("Are you sure you want to delete this topic?")
s.template = "cbi/tblsection"
s.template_addremove = "mqtt_sub/add_topic"
s.extedit = luci.dispatcher.build_url("admin/services/mqtt/subscriber/topics/%s")

s.create = function(self)
    local ret
    local exists = false
    local topic = self.map:formvalue("_topic_name")
    self.map.uci:foreach(self.config, self.sectiontype, function(s)
        if s.topic and s.topic == topic then
            exists = true
            return false
        end
    end)

    if exists then
        self.map:error_msg(translatef("Topic '%s' already exists", topic))
        return false, "error"
    end

    if not topic or #topic == 0 then
        local err_msg = translate("Incorrect topic.")
        self.map:error_msg(err_msg)
        return false
    end

    self.defaults = {
        topic = topic
	}
	
	local created = TypedSection.create(self)
    if created then
        self.map:set(created, "section_id", created)
		m.uci:commit(m.config)
		luci.http.redirect(
			ds.build_url("admin/services/mqtt/subscriber/topics/%s")
		)
	end
	return created
end

function s.remove(self, section)
    --when deleting topic, also delete all associated events
    if (section) then
        local section_id = self.map.uci:get(self.config, section, "section_id")
		if (section_id) then
			m.uci:foreach("mqtt_sub", section_id, function(s)
				m.uci:delete("mqtt_sub", section_id)
			end)
		end
    end
    
    m.uci:delete("mqtt_sub", section)
	m.uci:save("mqtt_sub")
    m.uci:commit("mqtt_sub")
    
    luci.http.redirect(ds.build_url("admin/services/mqtt/subscriber/topics"))
end

s:option( DummyValue, "topic", translate("Topic name"))

o = s:option( DummyValue, "qos", translate("QOS"), translate("QOS level"))
o.cfgvalue = function(self, section)
    local value = DummyValue.cfgvalue(self, section)

    return value and value or "-"
end

return m
