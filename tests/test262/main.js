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
"TestCases/ch09/9.9/S9.9_A4.js"
];

function myRunTest(test)
{
	var myTestException = undefined;

	if (ignore.indexOf(test.path) != -1)
		return;

	//if (test.path!="TestCases/ch10/10.1/S10.1.7_A1_T1.js")
	//	return;

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
