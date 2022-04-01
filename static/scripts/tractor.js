angular.module('tractor', ['rzSlider'])
.controller('TractorController', function ($scope, $http) {
    // initialize the content type of post request with text/plaien
    // TO AVOID triggering the pre-flight OPTIONS request
    $http.defaults.headers.post["Content-Type"] = "text/plain";

    // rzSlider - options: "onChange" - Function(sliderId, modelValue, highValue, pointerType): 

    var vm = this;

    $scope.hueSlider        = { minValue: 36, maxValue:  80, options: { floor: 0, ceil: 180, onChange: function(sliderId, modelValue, highValue, pointerType) { vm.applyChanges(null); } } };
    $scope.saturationSlider = { minValue: 80, maxValue: 255, options: { floor: 0, ceil: 255, onChange: function(sliderId, modelValue, highValue, pointerType) { vm.applyChanges(null); } } };
    
    $scope.maxMarkerSlider      = { value: 10, options: { floor: 1, ceil:  50 } };

    $scope.rowThresholdPxSlider = { value:   5, options: { floor: 1, ceil: 320,              onChange: function(sliderId, modelValue, highValue, pointerType) { vm.applyChanges(null); } } };
    $scope.rowSpacingPxSlider   = { value: 160, options: { floor: 10, ceil: 1000,             onChange: function(sliderId, modelValue, highValue, pointerType) { vm.applyChanges(null); } } };
    $scope.rowPerspectiveSlider = { value: 300, options: { floor: 0, ceil: 750,              onChange: function(sliderId, modelValue, highValue, pointerType) { vm.applyChanges(null); } } };

    $scope.data = {
        duration: 1.0,
        left: 12,
        right: 13,
        direction: 'center',
        url: 'http://10.3.141.165:8888/video',
        colorFilter: true,
        //detecting: false,
        colorFrom: '36,80,25',
        colorTo: '20,255,255',
        erode: 10,
        dilate: 10,
        contourMode: 'POLY',
        threshold: 5,
        maximumMarkers: 10
    };
    $scope.videoUrl = '/video?t=' + new Date().getTime();

    const handleResponse = function (response) {
        $scope.response = angular.toJson(response, true);
    };
    const applySliderValues = function () {
        $scope.data.colorFrom        = $scope.hueSlider.minValue + ',' + $scope.saturationSlider.minValue + ',25';
        $scope.data.colorTo          = $scope.hueSlider.maxValue + ',' + $scope.saturationSlider.maxValue + ',255';

        $scope.data.rowThresholdPx   = $scope.rowThresholdPxSlider.value;
        $scope.data.rowSpacingPx     = $scope.rowSpacingPxSlider.value;
        $scope.data.rowPerspectivePx = $scope.rowPerspectiveSlider.value;

        $scope.data.maximumMarkers   = $scope.maxMarkerSlider.value;
    };
    this.getData = function() {
      $http.get('/data').then(function (response) {
          console.log(response.data);
          let input = response.data.replace(/'/g, '"');
          input = input.replace(/\(/g, '[');
          input = input.replace(/\)/g, ']');
          input = input.replace(/T/g, 't');
          input = input.replace(/F/g, 'f');
          console.log('input: ', input);
          const data = angular.fromJson(input, true);
          console.log(data);
          $scope.hueSlider.minValue = data.colorfrom[0];
          $scope.hueSlider.maxValue = data.colorto[0];
          $scope.saturationSlider.minValue = data.colorfrom[1];
          $scope.saturationSlider.maxValue = data.colorto[1];
          $scope.thresholdSlider.value = data.threshold;
          $scope.maxMarkerSlider.value = data.maximumMarkers;
          $scope.data.detecting = data.detecting;
      });
    }
    this.stop = function () {
        this.applyChanges(false);
    };
    this.start = function () {
        this.applyChanges(true);
    };
    this.perform = function (direction) {
        $scope.data.direction = direction;
        $http.post('/perform', $scope.data).then(handleResponse);
    };
    this.applyChanges = function (detecting) {

        if (detecting === true) {
            $scope.data.detecting = "start";
        }
        else if (detecting === false){
            $scope.data.detecting = "stop";
        }
        else {
            $scope.data.detecting = "";
        }

        applySliderValues();

        $http.post('/applyChanges', $scope.data).then(handleResponse);
    };

    this.getData();
    console.log("initialize controller");
});
