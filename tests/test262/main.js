function myTest262(json)
{
	console.log("%s", json.testsCollection.name);
	json.testsCollection.tests.forEach(function(test){
		test.code = decode(test.code);
		myRunTest(test);
	})
}

var ignore = [
"TestCases/ch08/8.7/S8.7.1_A2.js",
"TestCases/ch08/8.7/S8.7_A5_T1.js",
"TestCases/ch08/8.5/8.5.1.js",
"TestCases/ch08/8.5/S8.5_A11_T1.js",
"TestCases/ch08/8.5/S8.5_A13_T2.js",
"TestCases/ch08/8.5/S8.5_A2.1.js",
"TestCases/ch08/8.5/S8.5_A2.2.js",
"TestCases/ch09/9.3/S9.3_A4.1_T1.js",
"TestCases/ch09/9.3/S9.3_A4.1_T2.js",
"TestCases/ch09/9.3/S9.3_A4.2_T1.js",
"TestCases/ch09/9.3/S9.3_A4.2_T2.js",
"TestCases/ch09/9.3/9.3.1/S9.3.1_A2.js",
"TestCases/ch09/9.3/9.3.1/S9.3.1_A3_T1.js",
"TestCases/ch09/9.3/9.3.1/S9.3.1_A3_T2.js",
"TestCases/ch09/9.3/9.3.1/S9.3.1_A4_T1.js",
"TestCases/ch09/9.3/9.3.1/S9.3.1_A4_T2.js",
"TestCases/ch09/9.3/9.3.1/S9.3.1_A5_T1.js",
"TestCases/ch09/9.3/9.3.1/S9.3.1_A5_T3.js",
"TestCases/ch09/9.3/9.3.1/S9.3.1_A6_T1.js",
"TestCases/ch09/9.3/9.3.1/S9.3.1_A6_T2.js",
"TestCases/ch09/9.8/9.8.1/S9.8.1_A10.js",
"TestCases/ch09/9.8/9.8.1/S9.8.1_A7.js",
"TestCases/ch09/9.8/9.8.1/S9.8.1_A8.js",
"TestCases/ch09/9.8/9.8.1/S9.8.1_A9_T1.js",
"TestCases/ch09/9.8/9.8.1/S9.8.1_A9_T2.js",
"TestCases/ch09/9.9/S9.9_A4.js",
"TestCases/ch10/10.5/10.5-1gs.js",
"TestCases/ch10/10.6/10.6-13-b-2-s.js",
"TestCases/ch10/10.6/10.6-13-b-3-s.js",
"TestCases/ch10/10.6/10.6-13-c-2-s.js",
"TestCases/ch10/10.6/10.6-13-c-3-s.js",
"TestCases/ch10/10.6/10.6-14-1-s.js",
"TestCases/ch10/10.6/10.6-14-b-1-s.js",
"TestCases/ch10/10.6/10.6-14-c-1-s.js",
"TestCases/ch10/10.6/10.6-5-1.js",
"TestCases/ch10/10.6/10.6-6-1.js",
"TestCases/ch10/10.6/10.6-6-2.js",
"TestCases/ch10/10.6/10.6-7-1.js",
"TestCases/ch10/10.6/S10.6_A2.js",
"TestCases/ch10/10.6/S10.6_A5_T1.js",
"TestCases/ch10/10.6/S10.6_A5_T4.js",
"TestCases/ch11/11.1/11.1.5/11.1.5-1-s.js",
"TestCases/ch11/11.1/11.1.5/11.1.5-1gs.js",
"TestCases/ch10/10.6/10.6-11-b-1.js"
];

function myRunTest(test)
{
	var myTestException = undefined;

	if (ignore.indexOf(test.path) != -1)
		return;

	//if (test.path!="TestCases/ch10/10.6/S10.6_A7.js")
	//	return;

	//console.log("%s", test.code);

	myTestException = runTest(test.code);

	console.log("TEST:%s", test.path);
	console.log("%s", test.description);

	var fail = false;

	if (test.negative != undefined) {
		var neg = test.negative === "NotEarlyError" ? test.negative :
			(test.negative === "^((?!NotEarlyError).)*$" ?
            test.negative : ".");

		if (myTestException == undefined) {
			console.log("FAILED: cannot get excetpion '%s'", neg);
			fail = true;
		} else if (!(new RegExp(neg, "i").test(myTestException))) {
			console.log("FAILED: expect exception '%s', but get '%s'", neg, myTestException);
			fail = true;
		}
	} else {
		if (myTestException != undefined) {
			console.log("FAILED: get exception '%s'", myTestException);
			fail = true;
		}
	}

	if (fail) {
		dump(test.code);
	} else {
		console.log("PASSED");
	}

	if (fail) {
		throw Error("test failed");
	}
}
