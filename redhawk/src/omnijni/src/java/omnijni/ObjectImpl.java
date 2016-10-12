/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

package omnijni;

public abstract class ObjectImpl extends org.omg.CORBA.portable.ObjectImpl
{
    protected ObjectImpl ()
    {
        this(0);
    }

    protected ObjectImpl (long ref)
    {
        this.ref_ = ref;
    }

    // Methods inherited from org.omg.CORBA.Object
    public boolean _is_a (String repoId)
    {
        return _is_a(_get_object_ref(), repoId);
    }

    public void _release ()
    {
        _release_ref();
    }

    public boolean _non_existent ()
    {
        return _non_existent(_get_object_ref());
    }

    // Methods specified to omnijni objects
    public void _set_object_ref (long ref)
    {
        _release_ref();
        this.ref_ = _narrow_object_ref(ref);
    }
    
    public long _get_object_ref ()
    {
        return _get_object_ref(this.ref_);
    }

    @Override
    protected void finalize ()
    {
        _release_ref();
    }

    private synchronized void _release_ref ()
    {
        if (this.ref_ != 0) {
            long ref = _get_object_ref();
            _delete_object_ref(ref);
            this.ref_ = 0;
        }
    }

    // Concrete subclasses must implement these effectively static methods.
    protected abstract long _narrow_object_ref (long ref);
    protected abstract long _get_object_ref (long ref);

    private static native boolean _is_a (long ref, String repoId);
    private static native boolean _non_existent (long ref);

    private static native void _delete_object_ref (long ref);

    protected long ref_;
}
