angular.module('tractor',[])
.controller('TractorController', function($scope, $http) {
	// initialize the content type of post request with text/plaien
	// TO AVOID triggering the pre-flight OPTIONS request
	$http.defaults.headers.post["Content-Type"] = "text/plain";
	
	const host = 'http://10.3.141.1:8080';
	$scope.showSnapShot = false;
	$scope.data = {
			duration: 1,
			left: 12,
			right: 13,
			direction: 'center',
			url: 'http://10.3.141.165:8888',
			cannyLowThreshold: 250,
			cannyHighThreshold: 350,
			houghThreshold: 15,
			houghMinLineLength: 20,
			houghMaxLineGap: 20
	};
	$scope.videoUrl = $scope.data.url + '/video';	
	
    this.startStop = function() {
		$http.get('webservice.php')
			.then(function(response) {
				$scope.response = angular.toJson(response, true);
			}, function(response) {
				$scope.response = angular.toJson(response, true);
			});
	};
    this.perform = function(direction) {
    	$scope.data.direction = direction;
    	console.log("perform: ", $scope.data);
		$http.post(host + '/perform', $scope.data)
			.then(function(response) {
				$scope.response = angular.toJson(response, true);
			}, function(response) {
				$scope.response = angular.toJson(response, true);
			});
	};
    this.streamVideo = function() {		
		$scope.showSnapShot = false;		
		$http.post(host + '/videoOnOff', $scope.data)
			.then(function(response) {
				$scope.videoUrl = host + '/video?t=' + new Date().getTime(); 
				$scope.response = angular.toJson(response, true);
			}, function(response) {
				$scope.videoUrl = $scope.data.url + '/video';
				$scope.response = angular.toJson(response, true);
			});
	};
    this.snapShot = function() {
		$http.post(host + '/prepareImage', $scope.data)
			.then(function (response) {
				$scope.showSnapShot = true;
				// add the date to the end of the image to ensure the broser to reload the image
				$scope.snapShotUrl = 'tmp/snapShot.jpg?t=' + new Date().getTime(); 
				$scope.response = angular.toJson(response, true);
			}, function(response) {
				$scope.response = angular.toJson(response, true);
			});	
	};

    console.log("initialize controller");
});
