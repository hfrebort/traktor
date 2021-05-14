angular.module('tractor',['rzSlider'])
.controller('TractorController', function($scope, $http) {
	// initialize the content type of post request with text/plaien
	// TO AVOID triggering the pre-flight OPTIONS request
	$http.defaults.headers.post["Content-Type"] = "text/plain";
	$scope.colorSlider = {
		minValue: 30,
		maxValue: 110,
		options: {
			floor: 0,
			ceil: 255
		}
	}
	$scope.thresholdSlider = {
		value: 5,
		options: {
			floor: 2,
			ceil: 20
		}
	}
	$scope.data = {
		duration: 0.1,
		left: 12,
		right: 13,
		direction: 'center',
		url: 'http://10.3.141.165:8888/video',
		colorFilter: true,
		detecting: false,
		colorFrom: '36,25,25',
		colorTo: '110,255,255',
		erode: 10,
		dilate: 20,
		contourMode: 'CONT',
		threshold: 5
	};
	$scope.videoUrl = '/video?t=' + new Date().getTime();

	const handleResponse = function(response) {
		$scope.response = angular.toJson(response, true);
	};
	const applySliderValues = function() {
		$scope.data.colorFrom = $scope.colorSlider.minValue + ',25,25';
		$scope.data.colorTo = $scope.colorSlider.maxValue + ',255,255';
		$scope.data.threshold = $scope.thresholdSlider.value;
	};
    this.stop = function() {
		this.applyChanges(false);
	};
    this.perform = function(direction) {
    	$scope.data.direction = direction;
		$http.post('/perform', $scope.data).then(handleResponse);
	};
    this.applyChanges = function(detecting) {
		$scope.data.detecting = detecting;
		applySliderValues();
		$http.post('/applyChanges', $scope.data).then(handleResponse);	
	};

    console.log("initialize controller");
});
