from rayoptics.environment import *

def SayaLens(gaps = [5,5,5], pupil=20, flds=[0,0.5]):
    opm = OpticalModel()
    opm.radius_mode = True
    
    osp = opm['optical_spec']
    osp['pupil'] = PupilSpec(osp, key=['object', 'pupil'], value=pupil)
    osp['fov'] = FieldSpec(osp, key=['object', 'angle'], value=20.0, flds=flds, is_relative=True)
    osp['wvls'] = WvlSpec([('F', 0.5), (587.5618, 1.0), ('C', 0.5)], ref_wl=1)
    
    sm = opm['seq_model']    
    # Object distance
    sm.gaps[0].thi = 1e10
    
   
#    opm.add_from_file('/Users/joshreich/src/lensStack/zmax_45169.zmx', t=gaps[0]) # 50x200
    opm.add_from_file('/Users/joshreich/src/lensStack/zmax_32921.zmx', t=gaps[0]) # 40x120 ACH                                     
    opm.flip(opm['ele_model'].elements[4])
    
    opm.add_from_file('/Users/joshreich/src/lensStack/right_lens.roa', t=gaps[1]) # -700 / -075 x 3 
    opm.flip(opm['ele_model'].elements[4])
    
    opm.add_from_file('/Users/joshreich/src/lensStack/zmax_49662.zmx', t=10) # 25x30 ASPH ACH VIS 0
#    opm.flip(opm['ele_model'].elements[8])
    
    opm.update_model()
    # Move the last element to the back focal length of the system
    sm.gaps[len(sm.gaps)-1].thi = opm['analysis_results']['parax_data'].fod.bfl
    opm.update_model()
    
    return opm



def evaluateLens(opm, preamble='', CSV=True):
    ar = opm['analysis_results']
    pm = opm['parax_model']
    ax_ray, pr_ray, fod = ar['parax_data']
    to_pkg = compute_third_order(opm)
    n_last = pm.sys[-1][mc.indx]
    u_last = ax_ray[-1][mc.slp]
    ta = to.seidel_to_transverse_aberration(to_pkg.loc['sum',:], n_last, u_last)
    
    if (CSV):
            print(preamble + ", " +
              str(ar['parax_data'].fod.efl) + ", " +
              str(ar['parax_data'].fod.bfl) + ", " +
              str(ar['parax_data'].fod.fno) + ", " +
              str(ta[0]) + ", " +
              str(ta[1]) + ", " +
              str(ta[2]) + ", " +
              str(ta[3]) + ", " +
              str(ta[4]) + ", " +
              str(ta[5])) 
    else:
            print ("EFL: " + str(ar['parax_data'].fod.efl))
            print ("BFL: " + str(ar['parax_data'].fod.bfl))
            print ("F/#: " + str(ar['parax_data'].fod.fno))
            print ("TSA: " + str(ta[0]))
            print ("TCO: " + str(ta[1]))
            print ("TAS: " + str(ta[2]))
            print ("SAS: " + str(ta[3]))
            print ("PTB: " + str(ta[4]))
            print ("DST: " + str(ta[5]))

g2 = 1
for g0 in range(30,200):
    for g1 in range(30,100):
        SayaModel = SayaLens(gaps=[g0*0.1,g1*0.1,g2], pupil=20)
        evaluateLens(SayaModel, str(g0*0.1) + ", " + str(g1*0.1) + ", " + str(g2))
