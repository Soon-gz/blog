function hookStart() {
    Java.perform(function() {
        var System = Java.use("java.lang.System");
        System.getProperty.overload("java.lang.String").implementation = function(arg) {
            return "Russia";
        }
        System.getenv.overload("java.lang.String").implementation = function(arg) {
            return "RkxBR3s1N0VSTDFOR180UkNIM1J9Cg==";
        }
    })
}

setImmediate(hookStart);