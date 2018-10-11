package emptyString.java;

import java.util.Properties;

public class EmptyString extends EmptyString_base {

    public EmptyString()
    {
        super();
        this.estr.setValue("ctor-value");
    }

    public void constructor()
    {
    }

    protected int serviceFunction() {
        return NOOP;
    }

    public static void configureOrb(final Properties orbProps) {
    }

}
