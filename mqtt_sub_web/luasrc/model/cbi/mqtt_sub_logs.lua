require("luci.config")

map = Map("mqtt_sub")

local s = map:section(Table, messages, translate("EVENTS LOG"), translate("The Events Log section contains a chronological list of various events related to the device."))
s.anonymous = true
s.template = "mqtt_sub/tblsection_eventlog"
s.addremove = false
s.refresh = true
s.table_config = {
    truncatePager = true,
    labels = {
        placeholder = "Search...",
        perPage = "Events per page {select}",
        noRows = "No entries found",
        info = ""
    },
    layout = {
        top = "<table><tr style='padding: 0 !important; border:none !important'><td style='display: flex !important; flex-direction: row'>{select}<span style='margin-left: auto; width:100px'>{search}</span></td></tr></table>",
        bottom = "{info}{pager}"
    }
}

local o2
o2 = s:option(DummyValue, "date", translate("Date"))
o2 = s:option(DummyValue, "sender", translate("Topic"))
o2= s:option(DummyValue, "event", translate("Message"))
s:option(DummyValue, "", translate(""))

return map