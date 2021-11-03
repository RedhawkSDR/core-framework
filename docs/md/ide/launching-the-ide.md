# Launching the REDHAWK IDE for the First Time

This section describes the basic process for starting the REDHAWK IDE for the first time.


> **NOTE**  
> Before starting the REDHAWK IDE for the first time, the REDHAWK Core Framework (CF) and the IDE must be installed ([Installation](../installation/_index.html#installing-redhawk-from-rpms)).  

1.  Start the REDHAWK IDE by entering the following command:

```bash
rhide
```

At startup, the IDE may prompt for a <abbr title="See Glossary.">workspace</abbr> location. The workspace stores many of the IDE's settings and also acts as a logical collection of projects under development. The IDE creates new projects in the workspace by default.


> **WARNING**  
> If you upgraded from 1.8.x, 1.9.x, or 1.10.x, it is recommended that you use a different workspace location rather than reusing a previous version's workspace. If you set the workspace to the same workspace that you used in 1.8.x, 1.9.x, or 1.10.x, you must delete the <abbr title="See Glossary.">domain</abbr> from <abbr title="See Glossary.">REDHAWK Explorer</abbr> before launching the domain from <abbr title="See Glossary.">Target SDR</abbr>.  
