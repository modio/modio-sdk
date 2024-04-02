package com.modio.modiosdk;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class Modio {
    private final Activity _context;

    public Modio(Activity context)
    {
        _context = context;
    }
    public String getCertificatePath()
    {
        copyAsset("modio.crt", _context.getFilesDir().getAbsolutePath() + "/" + "Certificates");

        return _context.getFilesDir().getAbsolutePath() + "/" + "Certificates/modio.crt";
    }

    public String getStorageDirectory()
    {
        return _context.getFilesDir().getAbsolutePath() + "/";
    }

    private void copyAsset(String asset, String to) {

        File dir = new File(to);
        if(!dir.exists()) {
            dir.mkdirs();
        }
        AssetManager assetManager = _context.getAssets();
        InputStream in = null;
        OutputStream out = null;
        try {
            in = assetManager.open(asset);
            File outFile = new File(to, asset);

            if(outFile.exists()) {
                Log.w("package:mine", "Certificate already exists: " + outFile.toString());
                return;
            }

            out = new FileOutputStream(outFile);
            copyFile(in, out);
            Log.w("package:mine", "Copied - Certificate Path: " + to + "/" + asset);
        }
        catch(IOException e) {
            e.printStackTrace();
            Log.d(asset, "Failed copying " + asset);
        }
        finally {
            if(in != null) {
                try {
                    in.close();
                }
                catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if(out != null) {
                try {
                    out.close();
                }
                catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }
    }
}
