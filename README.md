# tractor
A system to control a harrow
- The current view of camera
- Initialize harrow(drive to the middle)
- Adjust harrow left right
- Start/Stop automatic arrow adjustment

## setup raspberry ##
Use raspap.com to install an apache and install wifi connection
(https://www.youtube.com/watch?v=YbvSS8MJm2s)

## install tractor ##
- Copy the whole directory traktor to the home directory
- Start application which is a web.py-app with following command

python3 webservice.py

- After that it is available under (http://localhost:8080) 

## users ##
- ssh: pi, raspberry
- raspap: admin, secret
- wifi: raspi-webgui, ChangeMe

# Calculation
width (px) / height (cm) = factor (px/cm)
640 / 50 = 12,8 

## TODO's
- calculate and display frames per second

Phase 1:
- detect areas which are overloaded with weed and show a warning to handle it manually -> harrow is fixed

Phase 2:
- calculate the deviation for all rows -> corrected regarding the perspective 
