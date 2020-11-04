include $(TOPDIR)/rules.mk

PKG_NAME:=udpecho
PKG_VERSION:=1.0

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/udpecho
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Udp echo (using usock)
  DEPENDS:=
endef

define Package/udpecho/description
 An open source implementation of the server udp echo
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) \
		-o $(PKG_BUILD_DIR)/udpecho_server $(PKG_BUILD_DIR)/udpecho_server.c
endef


define Package/udpecho/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/udpecho_server $(1)/usr/sbin
endef

$(eval $(call BuildPackage,udpecho))
