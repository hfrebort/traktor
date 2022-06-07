angular.module('tractor', ['rzSlider'])
.controller('TractorController', function ($scope, $http) {
    // initialize the content type of post request with text/plaien
    // TO AVOID triggering the pre-flight OPTIONS request
    $http.defaults.headers.post["Content-Type"] = "text/plain";

    $scope.hueSlider        = { minValue: 36, maxValue:  80, options: { floor: 0, ceil: 179, onChange: function(s, m, h, p) { applyChanges(); } } };
    $scope.saturationSlider = { minValue:  0, maxValue: 180, options: { floor: 0, ceil: 255, onChange: function(s, m, h, p) { applyChanges(); } } };
    $scope.valueSlider      = { minValue:  0, maxValue: 180, options: { floor: 0, ceil: 255, onChange: function(s, m, h, p) { applyChanges(); } } };

    $scope.erodeSlider              = { value:   3, options: { floor: 0, ceil:   10,  onChange: function(s, m, h, p) { applyChanges(); } } };
    $scope.dilateSlider             = { value:   3, options: { floor: 0, ceil:   10,  onChange: function(s, m, h, p) { applyChanges(); } } };
    $scope.minimalContourAreaSlider = { value: 130, options: { floor: 0, ceil: 1000,  onChange: function(s, m, h, p) { applyChanges(); } } };
    
    $scope.maxRowSlider         = { value:   0, options: { floor:  0, ceil:   10, onChange: function(s, m, h, p) { applyChanges(); } } };
    $scope.rowThresholdPxSlider = { value:   5, options: { floor:  1, ceil:  320, onChange: function(s, m, h, p) { applyChanges(); } } };
    $scope.rowSpacingPxSlider   = { value: 160, options: { floor: 10, ceil: 1000, onChange: function(s, m, h, p) { applyChanges(); } } };
    $scope.rowPerspectiveSlider = { value: 200, options: { floor:  0, ceil:  300, onChange: function(s, m, h, p) { applyChanges(); } } };
    $scope.showMenu = true;

    $scope.data = {
        detecting: "start"
    }
    $scope.videoUrl = '/video';

    const handleResponse = function (response) {
        $scope.response = angular.toJson(response.config.data, true);
    };
    const applySliderValues = function () {
        $scope.data.colorFrom          = [ $scope.hueSlider.minValue, $scope.saturationSlider.minValue, $scope.valueSlider.minValue ];
        $scope.data.colorTo            = [ $scope.hueSlider.maxValue, $scope.saturationSlider.maxValue, $scope.valueSlider.maxValue ];

        $scope.data.erode              = $scope.erodeSlider.value;
        $scope.data.dilate             = $scope.dilateSlider.value;
        $scope.data.minimalContourArea = $scope.minimalContourAreaSlider.value;

        $scope.data.maxRows            = $scope.maxRowSlider.value;
        $scope.data.rowThresholdPx     = $scope.rowThresholdPxSlider.value;
        $scope.data.rowSpacingPx       = $scope.rowSpacingPxSlider.value;
        $scope.data.rowPerspectivePx   = $scope.rowPerspectiveSlider.value;
    };
    const applyChanges = function () {
        // row threshold maximum can only be the half of the row distance
        $scope.rowThresholdPxSlider.options.ceil = Math.floor($scope.rowSpacingPxSlider.value / 2);

        applySliderValues();

        $http.post('/applyChanges', $scope.data).then(handleResponse);
    };
    const loadSettingsFromBackend = function() {
        $http.get('/current').then(function(response) {
            let data = response.data;
            console.log('current settings from backend:', data);

            $scope.hueSlider.minValue              = data.colorFrom[0];
            $scope.hueSlider.maxValue              = data.colorTo[0];
            $scope.saturationSlider.minValue       = data.colorFrom[1];
            $scope.saturationSlider.maxValue       = data.colorTo[1];
            $scope.valueSlider.minValue            = data.colorFrom[2];
            $scope.valueSlider.maxValue            = data.colorTo[2];
                  
            $scope.erodeSlider.value               = data.erode;
            $scope.dilateSlider.value              = data.dilate;
            $scope.minimalContourAreaSlider.value  = data.minimalContourArea;

            $scope.maxRowSlider.value              = data.maxRows;
            $scope.rowSpacingPxSlider.value        = data.rowSpacingPx;
            $scope.rowPerspectiveSlider.value      = data.rowPerspectivePx;
            $scope.rowThresholdPxSlider.value      = data.rowThresholdPx;
        });
    };
    this.displayMenu = function() {
        $scope.showMenu = !$scope.showMenu;
    };

    loadSettingsFromBackend();
    console.log("controller initialized");
});