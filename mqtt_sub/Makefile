include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=mqtt_sub
PKG_VERSION:=1


include $(INCLUDE_DIR)/package.mk

define Package/mqtt_sub
	SECTION:=net
	CATEGORY:=Network
	TITLE:=MQTT subscriber
	DEPENDS:=+libmosquitto +libuci +libsqlite3 +libcurl +libjson-c
endef

define Package/mqtt_sub/description
	MQTT subscriber
endef

define Package/mqtt_sub/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/log
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mqtt_sub $(1)/usr/bin
	$(INSTALL_BIN) ./files/mqtt_sub.init $(1)/etc/init.d/mqtt_sub
	$(INSTALL_BIN) ./files/mqtt_sub.db $(1)/log
endef

$(eval $(call BuildPackage,mqtt_sub))