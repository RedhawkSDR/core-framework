package org.ossie.redhawk;

public class NetworkContainer {
    protected String _nic;
    
    public NetworkContainer() {
    }
    
    public NetworkContainer(String nic) {
        this._nic = nic;
    }
    
    public String getNic() {
        return this._nic;
    }
};
