function fail()
{
	throw Error("test failed");
}

function constructor_test()
{
	var d, t, now;
	
	now = Date();
	console.log("now: %s", now);

	if (typeof now != "string")
		fail();

	t = Date.parse(now);
	console.log("time: %f", t);

	if (typeof t != "number")
		fail();

	d = new Date(t);

	if (d.getTime() != t)
		fail();

	console.log("constructor test passed");
}

constructor_test();

function prototype_test()
{
	var d = new Date();

	function test(name, fmt) {
		var r = eval("d."+name+"()");
		console.log("%s: %"+fmt, name, r);
	}

	function test_s(name) {
		test(name, "s");
	}

	function test_tostr(name) {
		test(name, "s");
		var t = eval("d."+name+"()");
		var date = new Date(t);

		if (d.getTime() != date.getTime())
			fail();
	}

	function test_f(name) {
		test(name, "f");
	}

	function test_d(name) {
		test(name, "d");
	}

	function test_field(name) {
		test_d("get"+name);
		test_d("getUTC"+name);
	}

	test_s("toDateString");
	test_s("toTimeString");
	test_s("toLocaleDateString");
	test_s("toLocaleTimeString");
	test_s("toUTCString");
	test_s("toISOString");
	test_s("toJSON");

	test_tostr("toString");
	test_tostr("toLocaleString");

	test_f("valueOf");

	test_field("FullYear");
	test_field("Month");
	test_field("Date");
	test_field("Day");
	test_field("Hours");
	test_field("Minutes");
	test_field("Seconds");
	test_field("Milliseconds");
	test_d("getTimezoneOffset");

	var t = d.getTime();
	d = new Date();
	d.setTime(t);

	if (d.getTime() != t)
		fail();

	d.setFullYear(1978, 9, 9);
	if (d.getFullYear()!=1978 || d.getMonth()!=9 || d.getDate()!=9)
		fail();

	d.setFullYear(1982, 7);
	if (d.getFullYear()!=1982 || d.getMonth()!=7 || d.getDate()!=9)
		fail();

	d.setFullYear(1970);
	if (d.getFullYear()!=1970 || d.getMonth()!=7 || d.getDate()!=9)
		fail();

	d.setMonth(0, 31);
	if (d.getMonth()!=0 || d.getDate()!=31)
		fail();

	d.setMonth(0);
	if (d.getMonth()!=0)
		fail();


	d.setHours(7,30,59,999);
	console.log("%s", d.toString());
	if (d.getHours()!=7 || d.getMinutes()!=30 || d.getSeconds()!=59 || d.getMilliseconds()!=999)
		fail();

	d.setHours(8,45,59);
	if (d.getHours()!=8 || d.getMinutes()!=45 || d.getSeconds()!=59 || d.getMilliseconds()!=999)
		fail();

	d.setHours(9,50);
	if (d.getHours()!=9 || d.getMinutes()!=50 || d.getSeconds()!=59 || d.getMilliseconds()!=999)
		fail();

	d.setHours(10);
	if (d.getHours()!=10 || d.getMinutes()!=50 || d.getSeconds()!=59 || d.getMilliseconds()!=999)
		fail();

	d.setMinutes(19, 44, 888);
	if (d.getMinutes()!=19 || d.getSeconds()!=44 || d.getMilliseconds()!=888)
		fail();

	d.setMinutes(20, 58);
	if (d.getMinutes()!=20 || d.getSeconds()!=58 || d.getMilliseconds()!=888)
		fail();

	d.setMinutes(22);
	if (d.getMinutes()!=22 || d.getSeconds()!=58 || d.getMilliseconds()!=888)
		fail();

	d.setSeconds(59,777);
	if (d.getSeconds()!=59 || d.getMilliseconds()!=777)
		fail();

	d.setSeconds(12);
	if (d.getSeconds()!=12 || d.getMilliseconds()!=777)
		fail();

	d.setMilliseconds(999);
	if (d.getMilliseconds()!=999)
		fail();

	console.log("prototype test passed");
}

prototype_test();

console.log("all tests passed");
