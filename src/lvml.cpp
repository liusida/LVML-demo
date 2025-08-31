#include "lvml.h"

// Initialize static member
LVML* LVML::mInstance = nullptr;

LVML::LVML() {
  mServerUrl = "";
  mCurrentUrl = "";
  mCurrentUi = nullptr;
  screen_counter = 0;
}

LVML::~LVML() {
  if (mCurrentUi) {
    lv_obj_del(mCurrentUi);
  }
  
  // Clean up downloaded image descriptors
  cleanupImageDescriptors();
}

void LVML::begin() {
  // Set this instance as the current one for callbacks
  setInstance(this);
  
  lv_xml_register_event_cb(NULL, "load_screen", loadScreenCallback);
}

void LVML::setInstance(LVML* instance) {
  mInstance = instance;
}

// Generic screen loading callback function
void LVML::loadScreenXml(String xmlContent) {
  if (xmlContent.length() > 0) {
    // Find src attributes and download images, replace src with descriptor name
    xmlContent = preprocessXmlForImages(xmlContent);

    // Generate a unique name for the component
    String componentName = "screen_" + String(screen_counter++);
    
    // Register the new component
    lv_xml_component_register_from_data(componentName.c_str(), xmlContent.c_str());
    
    // Remove the current UI
    if (mCurrentUi) {
      lv_obj_del(mCurrentUi);
    }
    
    // Create the new UI
    mCurrentUi = (lv_obj_t *)lv_xml_create(lv_scr_act(), componentName.c_str(), NULL);
    if (mCurrentUi) {
      Serial.printf("Screen loaded successfully!\n");
    } else {
      Serial.printf("Failed to create screen\n");
    }
    
    onLoadScreen();

  } else {
    Serial.printf("Failed to load from server\n");
  }
}
void LVML::loadScreenUrl(String url) {
  // Store the current URL for relative path resolution
  mCurrentUrl = url;
  
  // get server url from url
  mServerUrl = url.substring(0, url.indexOf("/", 8));
  Serial.printf("Server URL: %s\n", mServerUrl.c_str());

  String xmlContent = loadXMLFromURL(url);
  loadScreenXml(xmlContent);
}

String LVML::loadXMLFromURL(String url) {
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    return "";
  }
  String xmlContent = http.getString();
  http.end();
  return xmlContent;
}

void LVML::onLoadScreen() {
  Serial.println("On load screen");
}

String LVML::preprocessXmlForImages(String xmlContent) {
  Serial.println("Preprocessing XML for images...");
  
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError parseResult = doc.Parse(xmlContent.c_str());
  
  if (parseResult != tinyxml2::XML_SUCCESS) {
    Serial.printf("XML parsing failed: %s\n", doc.ErrorStr());
    return xmlContent; // Return original if parsing fails
  }
  
  tinyxml2::XMLElement* root = doc.RootElement();
  if (!root) {
    Serial.println("No root element found in XML");
    return xmlContent;
  }
  
  // Find all lv_image elements recursively
  processImageElements(root);
  
  // Convert back to string
  tinyxml2::XMLPrinter printer;
  doc.Print(&printer);
  return String(printer.CStr());
}

void LVML::processImageElements(tinyxml2::XMLElement* element) {
  if (!element) return;
  
  // Check if current element is lv_image
  if (strcmp(element->Name(), "lv_image") == 0) {
    const char* src = element->Attribute("src");
    if (src) {
      String srcUrl = String(src);
      Serial.printf("Found image source: %s\n", srcUrl.c_str());
      
      // Resolve the URL
      String fullUrl = resolveImageUrl(srcUrl);
      
      // Download the image and get descriptor
      lv_image_dsc_t *imgDesc = downloadImageToDescriptor(fullUrl);
      
      if (imgDesc) {
        // Store the descriptor with a unique name
        String descName = generateImageDescriptorName(fullUrl);
        mImageDescriptors[descName] = imgDesc;
        
        // Replace the src attribute with the descriptor name
        element->SetAttribute("src", descName.c_str());
        
        // Register the image with LVGL's XML system so it can be found
        lv_xml_register_image(NULL, descName.c_str(), imgDesc);
        
        Serial.printf("Successfully downloaded and stored image: %s as %s\n", fullUrl.c_str(), descName.c_str());
      } else {
        Serial.printf("Failed to download image: %s\n", fullUrl.c_str());
      }
    }
  }
  
  // Recursively process all children
  for (tinyxml2::XMLElement* child = element->FirstChildElement(); 
       child; child = child->NextSiblingElement()) {
    processImageElements(child);
  }
}

String LVML::resolveImageUrl(const String &src) {
  // If it's already a full URL, return as is
  if (src.startsWith("http://") || src.startsWith("https://")) {
    return src;
  }
  
  // If it's a relative path, resolve against server URL
  if (mServerUrl.length() > 0) {
    if (src.startsWith("/")) {
      return mServerUrl + src;
    } else {
      // Relative to current URL
      int lastSlash = mCurrentUrl.lastIndexOf('/');
      if (lastSlash > 0) {
        return mCurrentUrl.substring(0, lastSlash + 1) + src;
      } else {
        return mServerUrl + "/" + src;
      }
    }
  }
  
  return src;
}

lv_image_dsc_t* LVML::downloadImageToDescriptor(const String &url) {
  HTTPClient http;
  http.begin(url);
  
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP GET failed for image, error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
    return nullptr;
  }
  
  // Get the image data
  int contentLength = http.getSize();
  if (contentLength <= 0) {
    Serial.println("Invalid content length for image");
    http.end();
    return nullptr;
  }
  
  // Allocate memory for the image data
  uint8_t *imageData = (uint8_t*)malloc(contentLength);
  if (!imageData) {
    Serial.println("Failed to allocate memory for image");
    http.end();
    return nullptr;
  }
  
  // Read the image data
  WiFiClient *stream = http.getStreamPtr();
  int bytesRead = 0;
  while (http.connected() && bytesRead < contentLength) {
    size_t available = stream->available();
    if (available > 0) {
      int toRead = min(available, (size_t)(contentLength - bytesRead));
      int read = stream->readBytes(imageData + bytesRead, toRead);
      bytesRead += read;
    }
    delay(1); // Small delay to prevent watchdog issues
  }
  
  http.end();
  
  if (bytesRead != contentLength) {
    Serial.printf("Incomplete image download: %d/%d bytes\n", bytesRead, contentLength);
    free(imageData);
    return nullptr;
  }
  
  // Create LVGL image descriptor
  lv_image_dsc_t *imgDesc = (lv_image_dsc_t*)malloc(sizeof(lv_image_dsc_t));
  if (!imgDesc) {
    Serial.println("Failed to allocate memory for image descriptor");
    free(imageData);
    return nullptr;
  }
  
  // Initialize the descriptor
  memset(imgDesc, 0, sizeof(lv_image_dsc_t));
  imgDesc->data = imageData;
  imgDesc->data_size = contentLength;
  
  // Set image header properties for PNG
  // LVGL 9.x header structure
  imgDesc->header.magic = LV_IMAGE_HEADER_MAGIC;
  imgDesc->header.cf = LV_COLOR_FORMAT_RAW_ALPHA;  // PNG format
  imgDesc->header.flags = 0;
  imgDesc->header.w = 320;  // Default width - ideally should be extracted from PNG
  imgDesc->header.h = 240;  // Default height - ideally should be extracted from PNG
  imgDesc->header.stride = 0;  // Let LVGL calculate this
  imgDesc->header.reserved_2 = 0;
  
  Serial.printf("Image downloaded successfully: %d bytes\n", contentLength);
  Serial.printf("Image descriptor created: %dx%d, format: %d\n", 
                imgDesc->header.w, imgDesc->header.h, imgDesc->header.cf);
  return imgDesc;
}



void LVML::downloadImagesFromXml(String xmlContent) {
  // This method will be called to download all images found in the XML
  // It's a placeholder for the image downloading functionality
  Serial.println("Downloading images from XML...");
}

String LVML::generateImageDescriptorName(const String &url) {
  // Generate a unique name for the image descriptor
  // Use a hash of the URL to create a unique identifier
  unsigned long hash = 0;
  for (size_t i = 0; i < url.length(); i++) {
    hash = ((hash << 5) + hash) + url.charAt(i); // Simple hash function
  }
  return "img_" + String(hash, HEX);
}

void LVML::cleanupImageDescriptors() {
  // Free all downloaded image descriptors and their data
  for (auto& pair : mImageDescriptors) {
    if (pair.second) {
      if (pair.second->data) {
        free((void*)pair.second->data);
      }
      free(pair.second);
    }
  }
  mImageDescriptors.clear();
  Serial.println("Cleaned up image descriptors");
}

//--------------------------------
// Callback function for loading screens
// Static callback that can access instance data
//--------------------------------
void LVML::loadScreenCallback(lv_event_t * e) {
  if (!mInstance) {
    Serial.println("No LVML instance available!");
    return;
  }
  
  const char * target_data = (const char *)lv_event_get_user_data(e);
  if (!target_data) {
    Serial.println("No target specified!");
    return;
  }
  
  Serial.printf("Loading target: %s\n", target_data);
  
  String fullUrl;
  String target(target_data);
  
  // Check if target is an absolute URL
  if (target.startsWith("http://") || target.startsWith("https://")) {
    fullUrl = target;
  } else {
    // Handle relative path
    if (mInstance->mCurrentUrl.length() > 0) {
      // If we have a current URL, resolve relative to it
      if (target.startsWith("/")) {
        // Absolute path from server root
        fullUrl = mInstance->mServerUrl + target;
      } else {
        // Relative path from current URL
        int lastSlash = mInstance->mCurrentUrl.lastIndexOf('/');
        if (lastSlash > 0) {
          fullUrl = mInstance->mCurrentUrl.substring(0, lastSlash + 1) + target;
        } else {
          fullUrl = mInstance->mServerUrl + "/" + target;
        }
      }
    } else {
      // Fallback to server URL + target
      fullUrl = mInstance->mServerUrl + "/" + target;
    }
  }
  
  mInstance->loadScreenUrl(fullUrl);
}
