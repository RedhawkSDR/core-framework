OSSIEHOME=/usr/local/redhawk/core

lc = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

all:
	sudo cp ./$(call lc,${FILE}).py ${OSSIEHOME}/lib/python/ossie/utils/sandbox/
	sudo chown root:root ${OSSIEHOME}/lib/python/ossie/utils/sandbox/$(call lc,${FILE}).py
	sudo -E ./build.py ${FILE}

clean:
	rm -rf *.pyc

distclean: clean
