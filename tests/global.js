function fail()
{
	throw Error("test failed");
}

function const_test()
{
	if (!isNaN(NaN))
		fail();

	if (isFinite(Infinity))
		fail();

	if (isFinite(-Infinity))
		fail();

	if (undefined != void 0)
		fail();

	console.log("const test passed");
}

const_test();

function parse_int_test()
{
	if (parseInt("19781009") != 19781009)
		fail();

	if (parseInt("-19781009") != -19781009)
		fail();

	if (parseInt("19781009a") != 19781009)
		fail();

	if (parseInt("  19781009e") != 19781009)
		fail();

	if (!isNaN(parseInt("asdd")))
		fail();

	if (parseInt("4294967296") != 4294967296)
		fail();

	if (parseInt("0x293efbca1") != 11071896737)
		fail();

	if (parseInt("-0x293efbca1") != -11071896737)
		fail();

	if (parseInt("0X293EFBCA1") != 11071896737)
		fail();

	if (parseInt("01234567654321") != 89755965649)
		fail();

	if (parseInt("-01234567654321") != -89755965649)
		fail();

	if (parseInt("0110010110001000111001110", 2) != 13308366)
		fail();

	if (parseInt("01abcdfghxyz", 36) != 4703759818572347)
		fail();

	if (parseInt("0x9876abcdef", 16) != 654825999855)
		fail();

	console.log("parse int test passed");
}

parse_int_test();

function parse_float_test()
{
	if (parseFloat("123456789.123") != 123456789.123)
		fail();

	if (parseFloat("   123456789.123") != 123456789.123)
		fail();

	if (parseFloat("123456789.123aaaa") != 123456789.123)
		fail();

	if (parseFloat("-123456789.123") != -123456789.123)
		fail();

	if (parseFloat(".12345") != 0.12345)
		fail();

	if (parseFloat("123456789.") != 123456789)
		fail();

	if (parseFloat("12345e-4") != 1.2345)
		fail();

	if (parseFloat("12345E-4") != 1.2345)
		fail();

	if (parseFloat("12345e10") != 12345e10)
		fail();

	if (parseFloat("12345e+10") != 12345e10)
		fail();

	if (!isNaN(parseFloat("A")))
		fail();

	console.log("parse float test passed");
}

parse_float_test();

function escape_test()
{
	function test(str, rstr)
	{
		if (escape(str) != rstr)
			fail();
	}

	test("a\n\r\t\v\f\\\"\'\hbc","a\\n\\r\\t\\v\\f\\\\\\\"\\\'hbc");
	test("\x80\u8882","\\x80\\u8882");

	function test(rstr, str)
	{
		if (unescape(str) != rstr)
			fail();
	}

	test("a\n\r\t\v\f\\\"\'\hbc","a\\n\\r\\t\\v\\f\\\\\\\"\\\'hbc");
	test("\x80\u8882","\\x80\\u8882");

	console.log("escape test passed");
}

escape_test();

function uri_test()
{
	function test(str, rstr)
	{
		if (encodeURI(str) != rstr)
			fail();
	}

	test("hello","hello");
	test(" 你好","%20%E4%BD%A0%E5%A5%BD");

	function test(rstr, str)
	{
		if (decodeURI(str) != rstr)
			fail();
	}

	test("hello","hello");
	test(" 你好","%20%E4%BD%A0%E5%A5%BD");

	function test(str, rstr)
	{
		if (encodeURIComponent(str) != rstr)
			fail();
	}

	test("://?#,你好","%3A%2F%2F%3F%23%2C%E4%BD%A0%E5%A5%BD");

	function test(rstr, str)
	{
		if (decodeURIComponent(str) != rstr)
			fail();
	}

	test("://?#,你好","%3A%2F%2F%3F%23%2C%E4%BD%A0%E5%A5%BD");

	console.log("uri test passed");
}

uri_test();

console.log("all tests passed");
