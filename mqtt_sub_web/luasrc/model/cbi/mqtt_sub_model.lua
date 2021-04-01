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

request = s:option(Value, "request", "Request topic", translate("Request topic (alphanumeric characters only)"))
request.default = "request"
request.datatype = "string"
request.maxlength = 65536

use_username_pw = s:option(Flag, "use_username_pw", "Use annonymous connection", "Select to use anonymous connection, otherwise set username and password")
use_username_pw.default = "1"
use_username_pw.rmempty = false

username = s:option(Value, "username", translate("Username"), translate("Specify username of remote host"))
username.datatype = "credentials_validate"
username.placeholder = translate("Username")
-- username:depends("use_username_pw", "0")
username.parse = function(self, section, novld, ...)
        local pass = luci.http.formvalue("cbid.mqtt_sub.config.password")
        local value = self:formvalue(section)
        if pass ~= nil and pass ~= "" and (value == nil or value == "") then
                self:add_error(section, "invalid", "Error: username is empty but password is not")
        end
        Value.parse(self, section, novld, ...)
end

password = s:option(Value, "password", translate("Password"), translate("Specify password of remote host. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
password.password = true
password.datatype = "credentials_validate"
password.placeholder = translate("Password")
-- password:depends("use_username_pw", "0")
password.parse = function(self, section, novld, ...)
        local user = luci.http.formvalue("cbid.mqtt_sub.config.username")
        local value = self:formvalue(section)
        if user ~= nil and user ~= "" and (value == nil or value == "") then
                self:add_error(section, "invalid", "Error: password is empty but username is not")
        end
        Value.parse(self, section, novld, ...)
end

---------------------------- Subscriber Settings ----------------------------

-- s2 = map:section(NamedSection, "config", "mqtt_sub", translate("Subscriber settings"), "")
-- s2.anonymous = true
-- FileUpload.unsafeupload = true

-- function s2.cfgsections(self)
--         return {"mqtt_sub"}
-- end
---------------------------- Security Tab ----------------------------

use_tls_ssl = s:option(Flag, "use_tls_ssl", translate("Use TLS/SSL"), translate("Mark to use TLS/SSL for connection"))
use_tls_ssl.default = "0"
use_tls_ssl.rmempty = false

tls_type = s:option(ListValue, "tls_type", translate("TLS Type"), translate("Select the type of TLS encryption"))
tls_type:depends({use_tls_ssl = "1"})
tls_type:value("cert", translate("Certificate based"))
tls_type:value("psk", translate("Pre-Shared-Key based"))

--- Certificates from browser ---

tls_insecure = s:option(Flag, "tls_insecure", translate("Allow insecure connection"), translate("Allow not verifying server authenticity"))
tls_insecure.default = "0"
tls_insecure.rmempty = false
tls_insecure:depends({use_tls_ssl="1", tls_type = "cert"})

local certificates_link = luci.dispatcher.build_url("admin", "system", "admin", "certificates")
o = s:option(Flag, "_device_sec_files", translate("Certificate files from device"), translatef("Choose this option if you want to select certificate files from device.\
Certificate files can be generated <a class=link href=%s>%s</a>", certificates_link, translate("here")))
o:depends({use_tls_ssl="1", tls_type = "cert"})

ca_file = s:option(FileUpload, "ca_file", translate("CA File"), translate("Upload CA file"));
ca_file:depends({use_tls_ssl="1", _device_sec_files="", tls_type = "cert"})

cert_file = s:option(FileUpload, "cert_file", translate("CERT File"), translate("Upload CERT file"));
cert_file:depends({use_tls_ssl="1", _device_sec_files="", tls_type = "cert"})

key_file = s:option(FileUpload, "key_file", translate("Key File"), translate("Upload Key file"));
key_file:depends({use_tls_ssl="1", _device_sec_files="", tls_type = "cert"})

--- Certificates from the device ---

ca_file = s:option(ListValue, "_device_ca_file", translate("CA File"), translate("Select CA file"));
ca_file:depends({use_tls_ssl="1", _device_sec_files="1", tls_type = "cert"})

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

cert_file = s:option(ListValue, "_device_cert_file", translate("CERT File"), translate("Select CERT file"));
cert_file:depends({use_tls_ssl="1", _device_sec_files="1", tls_type = "cert"})

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

key_file = s:option(ListValue, "_device_key_file", translate("Key File"), translate("Select Key file"));
key_file:depends({use_tls_ssl="1", _device_sec_files="1", tls_type = "cert"})

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

--- Pre shared key options ---

o = s:option(Value, "psk", translate("Pre-Shared-Key"), translate("The pre-shared-key in hex format with no leading “0x”"))
o.datatype = "lengthvalidation(0, 128)"
o.placeholder = "Key"
o:depends({use_tls_ssl="1", tls_type = "psk"})

o = s:option(Value, "identity", translate("Identity"), translate("Specify the Identity"))
o.datatype = "uciname"
o.placeholder = "Identity"
o:depends({use_tls_ssl="1", tls_type = "psk"})

--- Received message logs ---

local s2 = map:section(Table, messages, translate("EVENTS LOG"), translate("The Events Log section contains a chronological list of various events related to the device."))
s2.anonymous = true
s2.template = "mqtt_sub/tblsection_eventlog"
s2.addremove = false
s2.refresh = true
s2.table_config = {
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

o2 = s2:option(DummyValue, "date", translate("Date"))
o2 = s2:option(DummyValue, "sender", translate("Topic"))
o2= s2:option(DummyValue, "event", translate("Message"))
s2:option(DummyValue, "", translate(""))

return map