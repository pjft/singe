# Bytecode errors observed

The following errors were observed when attempting to run Time Gal and Ninja Hayate. \
Fixes were made to the underlying Lua code in order to allow the games to play.

Changes to ROM boot timing were also addressed, no changes to gameplay have been made.

The corrected `.singe` files are included here.


## Time Gal

     SINGE: Script Error!
     SINGE: error running function 'onOverlayUpdate': /home/user/.daphne/singe/timegal/timegal.singe:7771: attempt to perform arithmetic on a nil value
     SINGE: Script Error!
     Segmentation fault
     DaphneLoader failed with an unknown exit code : 139.

### Fix

     --- timegal.singe.decompiled    2019-12-14 01:04:29.345452887 +0000
     +++ timegal.singe       2019-12-14 00:54:28.698076588 +0000
     @@ -7768,7 +7768,10 @@
        local s1 = ""
        s1 = tostring(KqDPOaTfvHTF)
        for k = 1, string.len(s1) do
     -    j = tonumber(string.sub(s1, k, k)) + 1
     +    j = tonumber(string.sub(s1, k, k))
     +    if (j ~= nil) then 
     +       j = j + 1
     +    end
          spriteDraw(IUWIbuJzinwL + (k - 1) * 8, DpPBXJViXgVH, ZPehxjlAqzHd[j])
        end
      end


## Ninja Hayate

    SINGE: Script Error!
    SINGE: error running function 'onOverlayUpdate': /home/user/.daphne/singe/hayate/hayate.singe:6852: attempt to perform arithmetic on a nil value
    SINGE: Script Error!
    Segmentation fault
    DaphneLoader failed with an unknown exit code : 139.

### Fix

     --- hayate.singe.decompiled     2019-12-13 21:38:54.094021727 +0000
     +++ hayate.singe        2019-12-14 00:57:58.251435675 +0000
     @@ -6849,7 +6849,10 @@
        local TFXAtbKTzHng = ""
        TFXAtbKTzHng = tostring(yxqcFRAjsHNc)
        for k = 1, string.len(TFXAtbKTzHng) do
     -    j = tonumber(string.sub(TFXAtbKTzHng, k, k)) + 1
     +    j = tonumber(string.sub(TFXAtbKTzHng, k, k))
     +    if (j ~= nil) then
     +        j = j +1
     +    end 
          spriteDraw(RpMQMSEkgsbX + (k - 1) * 8, mWplUnviqWlO, xQzPvLUcOxgc[j])
        end
      end
