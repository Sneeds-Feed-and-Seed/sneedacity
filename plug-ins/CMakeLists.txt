
set( TARGET plug-ins )
set( TARGET_ROOT ${topdir}/plug-ins )

message( STATUS "========== Configuring ${TARGET} ==========" )

def_vars()

list( APPEND SOURCES
   SpectralEditMulti.ny
   SpectralEditParametricEQ.ny
   SpectralEditShelves.ny
   StudioFadeOut.ny
   adjustable-fade.ny
   beat.ny
   clipfix.ny
   crossfadeclips.ny
   crossfadetracks.ny
   delay.ny
   eq-xml-to-txt-converter.ny
   equalabel.ny
   highpass.ny
   label-sounds.ny
   limiter.ny
   lowpass.ny
   noisegate.ny
   notch.ny
   nyquist-plug-in-installer.ny
   pluck.ny
   rhythmtrack.ny
   rissetdrum.ny
   rms.ny
   sample-data-export.ny
   sample-data-import.ny
   spectral-delete.ny
   tremolo.ny
   vocalrediso.ny
   vocoder.ny
)

foreach( source ${SOURCES} )
   set( src "${_SRCDIR}/${source}" )
   set( dst "${_DEST}/${TARGET}/${source}" )

   add_custom_command(
      DEPENDS
         "${src}"
      COMMAND
         ${CMAKE_COMMAND} -E make_directory "${_DEST}/${TARGET}"
      COMMAND
         ${CMAKE_COMMAND} -E copy "${src}" "${dst}"
      OUTPUT
         "${dst}"
   )

   list( APPEND OUTPUTS "${dst}" )
endforeach()

add_custom_target( ${TARGET} ALL DEPENDS ${OUTPUTS} SOURCES ${SOURCES} )

if( NOT CMAKE_SYSTEM_NAME MATCHES "Darwin" )
   if( NOT WIN32 )
      install( DIRECTORY "${_DEST}/${TARGET}"
               DESTINATION "${_PKGDATA}" )
   endif()
endif()

