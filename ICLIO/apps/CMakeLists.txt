IF(QT_FOUND)
  ADD_SUBDIRECTORY(create)
  ADD_SUBDIRECTORY(reset-bus)
  ADD_SUBDIRECTORY(video-player)
  ADD_SUBDIRECTORY(multi-viewer)
ENDIF()

IF(QT_FOUND AND LIBDC_FOUND)
  ADD_SUBDIRECTORY(dcdeviceinfo)
  ADD_SUBDIRECTORY(dcclearisochannels)
  ADD_SUBDIRECTORY(reset-dc-bus)
ENDIF()

IF(LIBFREENECT2_FOUND)
  ADD_SUBDIRECTORY(k2)
ENDIF()

ADD_SUBDIRECTORY(camera-param-io)
ADD_SUBDIRECTORY(convert)
ADD_SUBDIRECTORY(jpg2cpp)
ADD_SUBDIRECTORY(pipe)
