angular.module('tractor', ['rzSlider'])
.controller('TractorController', function ($scope, $http) {
    // initialize the content type of post request with text/plaien
    // TO AVOID triggering the pre-flight OPTIONS request
    $http.defaults.headers.post["Content-Type"] = "text/plain";

    // rzSlider - options: "onChange" - Function(sliderId, modelValue, highValue, pointerType): 

    var vm = this;

    $scope.hueSlider = {
        minValue: 36,
        maxValue: 80,
        options: {
            floor: 0,
            ceil: 180
            //,onChange: applyChanges(true)
            ,onChange: function(sliderId, modelValue, highValue, pointerType) { vm.applyChanges(true); }
        }
    };
    $scope.saturationSlider = {
        minValue: 80,
        maxValue: 255,
        options: {
            floor: 0,
            ceil: 255
            //,onChange: applyChanges(true)
            ,onChange: function(sliderId, modelValue, highValue, pointerType) { vm.applyChanges(true); }
        }
    };
    $scope.thresholdSlider = {
        value: 5,
        options: {
            floor: 2,
            ceil: 20
        }
    };
    $scope.maxMarkerSlider = {
      value: 10,
      options: {
          floor: 1,
          ceil: 50
      }
    };
    $scope.rowCountSlider = {
        value: 1,
        options: {
            floor: 1,
            ceil: 7, 
            step: 2
            ,onChange: function(sliderId, modelValue, highValue, pointerType) { vm.applyChanges(true); }
        }
    };
    $scope.rowSpaceSlider = {
        value: 160,
        options: {
            floor: 0,
            ceil: 320
            ,onChange: function(sliderId, modelValue, highValue, pointerType) { vm.applyChanges(true); }
        }
    };
    $scope.rowPerspectiveSlider = {
        value: 0,
        options: {
            floor: 0,
            ceil: 500
            ,onChange: function(sliderId, modelValue, highValue, pointerType) { vm.applyChanges(true); }
        }
    };
    $scope.data = {
        duration: 1.0,
        left: 12,
        right: 13,
        direction: 'center',
        url: 'http://10.3.141.165:8888/video',
        colorFilter: true,
        detecting: false,
        colorFrom: '36,80,25',
        colorTo: '80,255,255',
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
        $scope.data.threshold        = $scope.thresholdSlider.value;
        $scope.data.maximumMarkers   = $scope.maxMarkerSlider.value;
        $scope.data.rowCount         = $scope.rowCountSlider.value;
        $scope.data.rowSpacePx       = $scope.rowSpaceSlider.value;
        $scope.data.rowPerspectivePx = $scope.rowPerspectiveSlider.value;
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
    this.perform = function (direction) {
        $scope.data.direction = direction;
        $http.post('/perform', $scope.data).then(handleResponse);
    };
    this.applyChanges = function (detecting) {
        $scope.data.detecting = detecting;
        applySliderValues();
        $http.post('/applyChanges', $scope.data).then(handleResponse);
    };

    this.getData();
    console.log("initialize controller");
});
