function myTest262(json)
{
	console.log("%s", json.testsCollection.name);
	json.testsCollection.tests.forEach(function(test){
		test.code = decode(test.code);
		myRunTest(test);
	})
}

var ignoreTests=[
	"TestCases/ch07/7.3/S7.3_A1.1_T1.js"
];

function myRunTest(test)
{
	var r = 0;

	try {
		eval(test.code);
	} catch (e) {
		if (ignoreTests.indexOf(test.path) != -1) {
			r = 1;
		}
	}

	if (r == 1) {
		dump(test.code);
	}

	console.log("%s: %s", test.description, r ? "failed" : "passed");

	if (r == 1)
		throw Error("test failed");
}
