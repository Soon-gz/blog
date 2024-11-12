'use strict';

console.log("Waiting for Java..");

Java.perform(function() {
    var process_clz = Java.use("android.os.Process");
	var Log = Java.use("android.util.Log");
    process_clz.killProcess.implementation = function(pid) {
        Log.e("fridaHook","kill process : " + pid);
    }

    var main_clz = Java.use("com.gdufs.xman.MyApp");
    main_clz.saveSN.implementation = function(str) {
        this.saveSN(str);
        Log.e("fridaHook","MyApp : "+str);
    }
})