angular.module('tractor',[])
.controller('TractorController', function($scope, $http) {
	// initialize the content type of post request with text/plaien
	// TO AVOID triggering the pre-flight OPTIONS request
	$http.defaults.headers.post["Content-Type"] = "text/plain";
	
	$scope.data = {
		duration: 1,
		left: 12,
		right: 13,
		direction: 'center',
		url: 'http://10.3.141.165:8888/video',
		colorFilter: true,
		detecting: true,
		colorFrom: '36,25,25',
		colorTo: '110,255,255',
		erode: 10,
		dilate: 20,
		contourMode: 'CONT',
		threshold: 30
	};
	$scope.videoUrl = $scope.data.url;
	
    this.stop = function() {
		$http.get('/stop')
			.then(function(response) {
				$scope.response = angular.toJson(response, true);
			}, function(response) {
				$scope.response = angular.toJson(response, true);
			});
	};
    this.perform = function(direction) {
    	$scope.data.direction = direction;
		$http.post('/perform', $scope.data)
			.then(function(response) {
				$scope.response = angular.toJson(response, true);
			}, function(response) {
				$scope.response = angular.toJson(response, true);
			});
	};
    this.streamVideo = function() {		
		$http.get('/videoOnOff', $scope.data)
			.then(function(response) {
				$scope.videoUrl = '/video?t=' + new Date().getTime(); 
				$scope.response = angular.toJson(response, true);
			}, function(response) {
				$scope.videoUrl = $scope.data.url;
				$scope.response = angular.toJson(response, true);
			});
	};
    this.applyChanges = function() {
		$http.post('/applyChanges', $scope.data)
			.then(function (response) {
				$scope.response = angular.toJson(response, true);
			}, function(response) {
				$scope.response = angular.toJson(response, true);
			});	
	};

    console.log("initialize controller");
});
