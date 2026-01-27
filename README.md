# ADAS_feedback_tool
A web app that any driver driving with ADAS features such as Adaptive Cruise Control, Lane Keeping Assist, and Traffic Sign Information can use to annotate their drive.

## Supported ADAS Feedback (v0)

This app allows drivers to log important ADAS moments with **one tap**, capturing subjective feedback that vehicle logs alone cannot provide.

### Adaptive Cruise Control (ACC)
- **Ghost brake**  
  The vehicle braked unexpectedly when no clear reason was visible.

- **Didn’t brake enough**  
  The vehicle did not decelerate as expected, and the driver had to brake.

### Lane Keep Assist (LKA)
- **Unexpected steering**  
  The vehicle applied steering intervention unexpectedly when no clear reason was visible.

### Speed Sign Recognition
- **Speed sign wrong**  
  The detected speed limit was incorrect.

Each tap automatically records the timestamp and GPS location.  
Additional context (such as road type) may be added later using open source map data.
