angular.module('tractor',[])
.controller('TractorController', function($scope, $http) {
	// initialize the content type of post request with text/plaien
	// TO AVOID triggering the pre-flight OPTIONS request
	$http.defaults.headers.post["Content-Type"] = "text/plain";
	
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
		threshold: 30
	};
	$scope.videoUrl = '/video?t=' + new Date().getTime();
	
    this.stop = function() {
		$scope.data.detecting = !$scope.data.detecting;
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
    this.applyChanges = function() {
		$http.post('/applyChanges', $scope.data)
			.then(function (response) {
				$scope.response = angular.toJson(response, true);
			}, function(response) {
				$scope.response = angular.toJson(response, true);
			});	
	};

    console.log("initialize controller");
}).directive('jsonText', function() {
    return {
        restrict: 'A',
        require: 'ngModel',
        link: function(scope, element, attr, ngModel) {            
          function into(input) {
            return JSON.parse(input);
          }
          function out(data) {
            return JSON.stringify(data);
          }
          ngModel.$parsers.push(into);
          ngModel.$formatters.push(out);

        }
    };
});
