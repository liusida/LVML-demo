#pragma once
#include <Arduino.h>
#include <lvgl.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <map>
#include <tinyxml2.h>

#include "misc/lv_types.h"
#include "others/xml/lv_xml_component.h"

class LVML {
  public:
    LVML(); // Constructor
    ~LVML(); // Destructor
    
    void begin();
    void loadScreenXml(String xmlContent);
    void loadScreenUrl(String url);
    String loadXMLFromURL(String url);
    
    // Static callback that can access instance data
    static void loadScreenCallback(lv_event_t * e);
    
    // Set the instance pointer for the callback to use
    static void setInstance(LVML* instance);

    void onLoadScreen();

  private:
    String mServerUrl;
    String mCurrentUrl;
    lv_obj_t *mCurrentUi;
    int screen_counter;
    
    // Static pointer to the current instance
    static LVML* mInstance;
    
    // Storage for downloaded image descriptors
    std::map<String, lv_image_dsc_t*> mImageDescriptors;
    
    // Helper methods for image handling
    lv_image_dsc_t* downloadImageToDescriptor(const String &url);
    void findAndProcessAllImages(lv_obj_t *parent);
    String resolveImageUrl(const String &src);
    String preprocessXmlForImages(String xmlContent);
    void downloadImagesFromXml(String xmlContent);
    String generateImageDescriptorName(const String &url);
    void cleanupImageDescriptors();
    void processImageElements(tinyxml2::XMLElement* element);
};
