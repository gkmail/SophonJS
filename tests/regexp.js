function fail()
{
	throw Error("test failed");
}

function constructor_test()
{
	var r;

	r = /./;
	if (r.source != ".")
		fail();
	if (r.toString() != "/./")
		fail();
	if (r.global)
		fail();
	if (r.ignoreCase)
		fail();
	if (r.multiline)
		fail();

	r = /./g;
	if (r.source != ".")
		fail();
	if (r.toString() != "/./g")
		fail();
	if (!r.global)
		fail();
	if (r.ignoreCase)
		fail();
	if (r.multiline)
		fail();

	r = /./m;
	if (r.source != ".")
		fail();
	if (r.toString() != "/./m")
		fail();
	if (r.global)
		fail();
	if (r.ignoreCase)
		fail();
	if (!r.multiline)
		fail();

	r = /./i;
	if (r.source != ".")
		fail();
	if (r.toString() != "/./i")
		fail();
	if (r.global)
		fail();
	if (!r.ignoreCase)
		fail();
	if (r.multiline)
		fail();

	r = /./igm;
	if (r.source != ".")
		fail();
	if (r.toString() != "/./igm")
		fail();
	if (!r.global)
		fail();
	if (!r.ignoreCase)
		fail();
	if (!r.multiline)
		fail();

	console.log("constructor test passed");
}

constructor_test();

function exec_test()
{
	function test(re, str, r, idx)
	{
		var m = re.exec(str);

		if ((r == null) && (m != null))
			fail();

		if ((r != null) && (m == null))
			fail();

		if (m == null)
			return;

		if (r.length != m.length)
			fail();

		if (m.index != idx)
			fail();

		for (var i = 0; i < r.length; i++){
			if (m[i] != r[i])
				fail();
		}
	}

	test(RegExp("//"),"abcd",null,0);
	test(/./,"abcde",["a"],0);
	test(/.+/,"abcde",["abcde"],0);
	test(/.{3}/,"abcde",["abc"],0);
	test(/[bc]{2}/,"abcde",["bc"],1);
	test(/[bc]{2,3}/,"abcde",["bc"],1);
	test(/[bc]{2,3}/,"abcbc",["bcb"],1);
	test(/[bc]{2,3}?/,"abcbc",["bc"],1);
	test(/\d+/,"abc1234ef",["1234"],3);
	test(/a|ab/,"abc",["a"],0);
	test(/((a)|(ab))((c)|(bc))/,"abc",["abc","a","a",undefined,"bc",undefined,"bc"],0);
	test(/a[a-z]{2,4}/,"abcdefghi",["abcde"],0);
	test(/a[a-z]{2,4}?/,"abcdefghi",["abc"],0);
	test(/(aa|aabaac|ba|b|c)*/,"aabaac",["aaba","ba"],0);
	test(/(z)((a+)?(b+)?(c))*/,"zaacbbbcac",["zaacbbbcac","z","ac","a",undefined,"c"],0);
	test(/(a*)b\1+/,"baaaac",["b",""],0);
	test(/(?=(a+))/,"baaabac",["","aaa"],1);
	test(/(?=(a+))a*b\1/,"baaabac",["aba","a"],3);
	test(/(.*?)a(?!(a+)b\2c)\2(.*)/,"baaabaac",["baaabaac", "ba", undefined, "abaac"], 0);
	test(/a/,"abc",["a"],0);
	test(/a(?!b)/,"abc",null,0);
	test(/^a/,"ba",null,0);
	test(/^a/,"aa",["a"],0);
	test(/a$/,"ab",null,0);
	test(/a$/,"aa",["a"],1);
	test(/\d+/,"a0123456789b",["0123456789"],1);
	test(/\D+/,"0a%@bc9",["a%@bc"],1);
	test(/\w+/,"$_12abc%",["_12abc"],1);
	test(/\W+/,"_12@#$avc",["@#$"],3);
	test(/\s+/,"abc \t\n\r 12",[" \t\n\r "],3);
	test(/\S+/," \thello\n\r ",["hello"],2);
	test(/\bhello\b/, "hello world", ["hello"], 0);
	test(/\bhello\b/, "helloworld", null);
	test(/\Bhello\B/, "hello", null);
	test(/\Bhello\B/, "ahellob", ["hello"], 1);
	test(/(?:abc)/, "1abc2", ["abc"], 1);
	test(/[a-g]+/, "12abcdefghi", ["abcdefg"], 2);
	test(/[^a-g]+/, "abcdefghiab", ["hi"], 7);
	test(/abc/i, "abc", ["abc"], 0);
	test(/abc/i, "aBc", ["aBc"], 0);
	test(/^a/, "ba\nab", null, 0);
	test(/^a/m, "ba\nab", ["a"], 3);
	test(/a$/, "ba\nab", null, 0);
	test(/a$/m, "ba\nab", ["a"], 1);

	var re = /\w+/g;
	var str = "hello world! sophon js!";

	test(re, str, ["hello"], 0);
	test(re, str, ["world"], 6);
	test(re, str, ["sophon"], 13);
	test(re, str, ["js"], 20);
	test(re, str, null, 0);
	test(re, str, ["hello"], 0);

	console.log("exec test passed");
}

exec_test();

function test_test()
{
	function test_true(re, str)
	{
		if (!re.test(str))
			fail();
	}

	function test_false(re, str)
	{
		if (re.test(str))
			fail();
	}

	test_true(/\d+/,"abcd123efg");
	test_true(/\w+/,"abcd123efg");
	test_false(/\s+/,"abcd123efg");

	console.log("test test passed");
}

test_test();

console.log("all tests passed");
