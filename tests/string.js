function fail()
{
	throw Error("test failed");
}

function constructor_test()
{
	var test=function(o, str)
	{
		if (o != str)
			fail();

		if (typeof o != "string")
			fail();
	}

	test(String(),"");
	test(String(""),"");
	test(String("sophon"),"sophon");
	test(String(123),"123");
	test(String(true),"true");
	test(String(false),"false");
	test(String(null),"null");
	test(String(undefined),"undefined");

	var test=function(o, str)
	{
		if (o != str)
			fail();

		if (!(o instanceof String))
			fail();

		if (!(o instanceof Object))
			fail();
	}

	test(new String,"");
	test(new String(),"");
	test(new String(""),"");
	test(new String("sophon"),"sophon");
	test(new String(123),"123");
	test(new String(true),"true");
	test(new String(false),"false");
	test(new String(null),"null");
	test(new String(undefined),"undefined");

	console.log("constructor test passed");
}

constructor_test();

function property_test()
{
	if ("".length != 0)
		fail();
	if ("a".length != 1)
		fail();
	if ("123".length != 3)
		fail();
	if ("hello sophon js".length != 15)
		fail();

	console.log("property test passed");
}

property_test();

function function_test()
{
	var str, i, r;

	str = "0123456789";
	for (i=0; i<10; i++){
		r = str.charAt(i);
		if (r != i.toString())
			fail();

		r = str.charCodeAt(i);
		if (r != i + 0x30)
			fail();
	}
	if (str.charAt(-1) != "")
		fail();
	if (str.charAt(10) != "")
		fail();
	if (!isNaN(str.charCodeAt(-1)))
		fail();
	if (!isNaN(str.charCodeAt(10)))
		fail();

	str = "hello ".concat("world",",","js","!");
	if (str != "hello world,js!")
		fail();
	str = "hello".concat(" sophon");
	if (str != "hello sophon")
		fail();
	str = "hello".concat();
	if (str != "hello")
		fail();

	if ("abcdefg".indexOf("def") != 3)
		fail();
	if ("abcdefg".indexOf("defgh") != -1)
		fail();
	if ("abcdefg".indexOf("def", -1) != 3)
		fail();
	if ("abcdefg".indexOf("def", 4) != -1)
		fail();
	if ("abcabc".indexOf("abc", 0) != 0)
		fail();
	if ("abcabc".indexOf("abc", 1) != 3)
		fail();
	if ("abcabc".indexOf("", 0) != 0)
		fail();

	if ("abcdefg".lastIndexOf("def") != 3)
		fail();
	if ("abcdefg".lastIndexOf("defgh") != -1)
		fail();
	if ("abcdefg".lastIndexOf("def", -1) != -1)
		fail();
	if ("abcdefg".lastIndexOf("def", 3) != 3)
		fail();
	if ("abcdefg".lastIndexOf("def", 2) != -1)
		fail();
	if ("abcabc".lastIndexOf("abc") != 3)
		fail();
	if ("abcabc".lastIndexOf("abc", 2) != 0)
		fail();
	if ("abcabc".lastIndexOf("") != 6)
		fail();

	if ("a".localeCompare("a") != 0)
		fail();
	if ("a".localeCompare("b") >= 0)
		fail();
	if ("a".localeCompare("c") >= 0)
		fail();
	if ("b".localeCompare("a") <= 0)
		fail();
	if ("a".localeCompare("") <= 0)
		fail();
	if ("".localeCompare("a") >= 0)
		fail();
	if ("".localeCompare("") != 0)
		fail();

	if ("0123456789".slice(0,0)!="")
		fail();
	if ("0123456789".slice(0,1)!="0")
		fail();
	if ("0123456789".slice(0,2)!="01")
		fail();
	if ("0123456789".slice(0,10)!="0123456789")
		fail();
	if ("0123456789".slice(0,11)!="0123456789")
		fail();
	if ("0123456789".slice(4,8)!="4567")
		fail();
	if ("0123456789".slice(1,0)!="")
		fail();
	if ("0123456789".slice(5,-1)!="5678")
		fail();
	if ("0123456789".slice(-6,-1)!="45678")
		fail();
	if ("0123456789".slice(-6,8)!="4567")
		fail();
	if ("0123456789".slice(-6,-7)!="")
		fail();

	if ("0123456789".substring(0,0)!="")
		fail();
	if ("0123456789".substring(0,1)!="0")
		fail();
	if ("0123456789".substring(0,2)!="01")
		fail();
	if ("0123456789".substring(0,10)!="0123456789")
		fail();
	if ("0123456789".substring(0,11)!="0123456789")
		fail();
	if ("0123456789".substring(4,8)!="4567")
		fail();
	if ("0123456789".substring(1,0)!="0")
		fail();
	if ("0123456789".substring(5,-1)!="01234")
		fail();
	if ("0123456789".substring(-6,-1)!="")
		fail();
	if ("0123456789".substring(-6,8)!="01234567")
		fail();
	if ("0123456789".substring(-6,-7)!="")
		fail();

	if ("".toLowerCase()!="")
		fail();
	if ("0123".toLowerCase()!="0123")
		fail();
	if ("abcdefg".toLowerCase()!="abcdefg")
		fail();
	if ("ABCDEFG".toLowerCase()!="abcdefg")
		fail();
	if ("ABcdeFG".toLowerCase()!="abcdefg")
		fail();

	if ("".toUpperCase()!="")
		fail();
	if ("0123".toUpperCase()!="0123")
		fail();
	if ("abcdefg".toUpperCase()!="ABCDEFG")
		fail();
	if ("ABCDEFG".toUpperCase()!="ABCDEFG")
		fail();
	if ("ABcdeFG".toUpperCase()!="ABCDEFG")
		fail();

	if ("".trim()!="")
		fail();
	if (" \n\r\tabcdefg \n\r".trim()!="abcdefg")
		fail();
	if (" \n\r\tabc \t\n\rdefg \n\r".trim()!="abc \t\n\rdefg")
		fail();

	if ("0123456789".substr(0,0)!="")
		fail();
	if ("0123456789".substr(0,1)!="0")
		fail();
	if ("0123456789".substr(0,2)!="01")
		fail();
	if ("0123456789".substr(3,5)!="34567")
		fail();
	if ("0123456789".substr(3,8)!="3456789")
		fail();
	if ("0123456789".substr(-5,3)!="567")
		fail();
	if ("0123456789".substr(-5,-1)!="")
		fail();


	console.log("function test passed");
}

function_test();

function match_test()
{
	function test(re, str, m, pos)
	{
		var r = str.match(re);

		if ((r == null) && (m == null))
			return;

		if ((r == null) || (m == null))
			fail();

		if (r.length != m.length)
			fail();

		for (var i = 0; i < r.length; i++) {
			if (r[i] != m[i])
				fail();
		}

		if (pos != -1){
			if (r.index != pos)
				fail();
		}
	}

	test(/\d+/,"abc123efg456abc789efg",["123"],3);
	test(/\d+/g,"abc123efg456abc789efg",["123","456","789"],-1);
	test(/\d+/,"abcefgabcefg",null,0);
	test("\\d+","abc123efg456abc789efg",["123"],3);
	test("\\d+","abcefgabcefg",null,0);
	test(/^\d+(?=:)/gm, "1:abc\n2:efg\n3:hij\n4a\na5",["1","2","3"],-1);
	test(/\w*/g, "hello sophon js", ["hello","","sophon","","js",""], -1);

	console.log("match test passed");
}

match_test();

function search_test()
{
	function test(re, str, r)
	{
		if (str.search(re) != r)
			fail();
	}

	test(/\d+/,"abc1234efg",3);
	test(/\d+x/,"abc1234efg",-1);
	test("\\d+","abc1234efg",3);
	test("\\d+x","abc1234efg",-1);

	console.log("search test passed");
}

search_test();

function replace_test()
{
	function test(str, re, rstr, r)
	{
		var rr = str.replace(re, rstr);
		if (r != rr) {
			console.log("\"%s\" \"%s\" %d", rr, r, rr==r);
			fail();
		}
	}

	test("sophon hello test", "hello", "js", "sophon js test");
	test("sophon hello test hellohello", "hello", "js", "sophon js test hellohello");
	test("sophon hello test", "hello", "$$$& js", "sophon $hello js test");
	test("sophon hello test", "hello", "$'js", "sophon  testjs test");
	test("sophon hello test", "hello", "$`js", "sophon sophon js test");
	test("sophon hello test", "hellox", "js", "sophon hello test");
	test("sophon hello test", "hello", "$1js", "sophon js test");

	test("sophon hello test", /\w+/, "x", "x hello test");
	test("sophon hello test", /\w+/g, "x", "x x x");

	test("a1 b2 c3", /(\w)(\d)/g, "$1$1$2$2", "aa11 bb22 cc33");
	test("var1=123 var2=456 var3=789", /(\w+)=(\d+)/g, "$1=\"$2\"($&)", "var1=\"123\"(var1=123) var2=\"456\"(var2=456) var3=\"789\"(var3=789)");

	test("hello TEST ok",/test/i,"sophon","hello sophon ok");

	test("hello sophon js",/\w*/g,"x","xx xx xx");

	console.log("replace test passed");
}

replace_test();

function split_test()
{
	function test(str, sep, lim, r)
	{
		var rr;

		if (sep == null)
			rr = str.split();
		else if (lim == null)
			rr = str.split(sep);
		else
			rr = str.split(sep, lim);

		if (r.length != rr.length)
			fail();

		for (var i = 0; i < r.length; i++)
			if (r[i] != rr[i])
				fail();
	}

	test("abcdefg", null, null, ["abcdefg"]);
	test("abcdefg", "", null, ["a","b","c","d","e","f","g"]);
	test("hello sophon js"," ",null,["hello","sophon","js"]);
	test("hello sophon js"," ",2,["hello","sophon"]);
	test("hello sophon js"," ",1,["hello"]);
	test("hello sophon js"," ",0,[]);
	test(" hello sophon js "," ",null,["","hello","sophon","js",""]);

	test("hello \tsophon\n\tjs",/\s+/,null,["hello","sophon","js"])
	test(" hello \tsophon\n\tjs\n",/\s+/,null,["","hello","sophon","js",""])

	console.log("split test passed");
}

split_test();

console.log("all test passed");
