require("luci.config")
local certs = require "luci.model.certificate"
local certificates = certs.get_certificates()
local keys = certs.get_keys()
local cas = certs.get_ca_files().certs

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

---------------------------- Subscriber Settings ----------------------------

local s2
s2 = map:section(NamedSection, "config", "mqtt_sub", translate("Security"), "")
-- s2.template = "mqtt/tsection"
-- s2.anonymous = true
-- FileUpload.unsafeupload = true
-- function s2.cfgsections(self)
--         return {"mqtt_sub"}
-- end

---------------------------- Security Tab ----------------------------

use_username_pw = s2:option(Flag, "use_username_pw", "Use annonymous connection", "Select to use anonymous connection, otherwise set username and password")
use_username_pw.default = "1"
use_username_pw.rmempty = false

username = s2:option(Value, "username", translate("Username"), translate("Specify username of remote host"))
username.datatype = "credentials_validate"
username.placeholder = translate("Username")
-- username:depends({use_username_pw="0"})
username.parse = function(self, section, novld, ...)
        local pass = luci.http.formvalue("cbid.mqtt_sub.config.password")
        local value = self:formvalue(section)
        if pass ~= nil and pass ~= "" and (value == nil or value == "") then
                self:add_error(section, "invalid", "Error: username is empty but password is not")
        end
        Value.parse(self, section, novld, ...)
end

password = s2:option(Value, "password", translate("Password"), translate("Specify password of remote host. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
password.password = true
password.datatype = "credentials_validate"
password.placeholder = translate("Password")
-- password:depends({use_username_pw="0"})
password.parse = function(self, section, novld, ...)
        local user = luci.http.formvalue("cbid.mqtt_sub.config.username")
        local value = self:formvalue(section)
        if user ~= nil and user ~= "" and (value == nil or value == "") then
                self:add_error(section, "invalid", "Error: password is empty but username is not")
        end
        Value.parse(self, section, novld, ...)
end

use_tls_ssl = s2:option(Flag, "use_tls_ssl", translate("Use TLS/SSL"), translate("Mark to use TLS/SSL for connection"))
use_tls_ssl.default = "0"
use_tls_ssl.rmempty = false

--- Certificates from browser ---

local certificates_link = luci.dispatcher.build_url("admin", "system", "admin", "certificates")
o = s2:option(Flag, "_device_sec_files", translate("Certificate files from device"), translatef("Choose this option if you want to select certificate files from device.\
Certificate files can be generated <a class=link href=%s>%s</a>", certificates_link, translate("here")))
o:depends({use_tls_ssl="1"})

FileUpload.size = "262144"
FileUpload.sizetext = translate("Selected file is too large, max 256 KiB")
FileUpload.sizetextempty = translate("Selected file is empty")
FileUpload.unsafeupload = true

ca_file = s2:option(FileUpload, "ca_file", translate("CA File"), translate("Upload CA file"));
ca_file:depends({use_tls_ssl="1", _device_sec_files=""})

cert_file = s2:option(FileUpload, "cert_file", translate("CERT File"), translate("Upload CERT file"));
cert_file:depends({use_tls_ssl="1", _device_sec_files=""})

key_file = s2:option(FileUpload, "key_file", translate("Key File"), translate("Upload Key file"));
key_file:depends({use_tls_ssl="1", _device_sec_files=""})

--- Certificates from the device ---

ca_file = s2:option(ListValue, "_device_ca_file", translate("CA File"), translate("Select CA file"));
ca_file:depends({use_tls_ssl="1", _device_sec_files="1"})

if #cas > 0 then
    for _,ca in pairs(cas) do
            ca_file:value("/etc/certificates/" .. ca.name, ca.name)
    end
else
    ca_file:value("", "-- No file available --")
end

function ca_file.write(self, section, value)
    map.uci:set(self.config, section, "ca_file", value)
end

ca_file.cfgvalue = function(self, section)
        return map.uci:get(map.config, section, "ca_file") or ""
end

cert_file = s2:option(ListValue, "_device_cert_file", translate("CERT File"), translate("Select CERT file"));
cert_file:depends({use_tls_ssl="1", _device_sec_files="1"})

if #certificates > 0 then
        for _,cert in pairs(certificates) do
                cert_file:value("/etc/certificates/" .. cert.name, cert.name)
        end
else
        cert_file:value("", "-- No file available --")
end

function cert_file.write(self, section, value)
        map.uci:set(self.config, section, "cert_file", value)
end

cert_file.cfgvalue = function(self, section)
        return map.uci:get(map.config, section, "cert_file") or ""
end

key_file = s2:option(ListValue, "_device_key_file", translate("Key File"), translate("Select Key file"));
key_file:depends({use_tls_ssl="1", _device_sec_files="1"})

if #keys > 0 then
        for _,key in pairs(keys) do
                key_file:value("/etc/certificates/" .. key.name, key.name)
        end
else
        key_file:value("", "-- No file available --")
end

function key_file.write(self, section, value)
        map.uci:set(self.config, section, "key_file", value)
end

key_file.cfgvalue = function(self, section)
        return map.uci:get(map.config, section, "key_file") or ""
end

--- Topic list ---

local s3 = map:section(TypedSection, "topic", translate("Topics"), translate(""))
s3.addremove = true
s3.anonymous = true
-- s3.template = "mqtt/tblsection"
s3.novaluetext = "There are no topics created yet."

topic = s3:option(Value, "topic", translate("Topic name"), translate(""))
topic.datatype = "string"
topic.maxlength = 65536
topic.rmempty = false
topic.parse = function(self, section, novld, ...)
        local value = self:formvalue(section)
        if value == nil or value == "" then
                self.map:error_msg(translate("Topic name can not be empty"))
                self.map.save = false
        end
        Value.parse(self, section, novld, ...)
end

qos = s3:option(ListValue, "qos", translate("QoS level"), translate("The publish/subscribe QoS level used for this topic"))
qos:value("0", "At most once (0)")
qos:value("1", "At least once (1)")
qos:value("2", "Exactly once (2)")
qos.rmempty=false
qos.default="0"

--- Received message logs ---

local s4 = map:section(Table, messages, translate("EVENTS LOG"), translate("The Events Log section contains a chronological list of various events related to the device."))
s4.anonymous = true
s4.template = "mqtt_sub/tblsection_eventlog"
s4.addremove = false
s4.refresh = true
s4.table_config = {
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
o2 = s4:option(DummyValue, "date", translate("Date"))
o2 = s4:option(DummyValue, "sender", translate("Topic"))
o2= s4:option(DummyValue, "event", translate("Message"))
s4:option(DummyValue, "", translate(""))

return map