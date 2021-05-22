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


## TODO's
- calculate and display frames per second
- https://angular-slider.github.io/angularjs-slider/

# Calculation
width (px) / height (cm) = factor (px/cm)
640 / 50 = 12,8 
