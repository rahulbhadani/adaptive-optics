/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.40
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package loci.hardware.camera.swig;

public class CCDCamWrapper {
  public static boolean init_camera() {
    return CCDCamWrapperJNI.init_camera();
  }

  public static String get_note() {
    return CCDCamWrapperJNI.get_note();
  }

  public static int capture_frame() {
    return CCDCamWrapperJNI.capture_frame();
  }

  public static short get_frame_at_pos(int index) {
    return CCDCamWrapperJNI.get_frame_at_pos(index);
  }

  public static int test_me() {
    return CCDCamWrapperJNI.test_me();
  }

}