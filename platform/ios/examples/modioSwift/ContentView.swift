//
//  ContentView.swift
//  modioSDK
//
//  Created by RH VT on 9/4/2024.
//

import SwiftUI

struct ModInfo: Equatable, Identifiable
{
    var id: ObjectIdentifier
    var name: String
    var desc: String
}

struct ContentView: View
{
    @State var modList: [ModInfo] = []
    
    var body: some View {
        VStack {
            List {
                ForEach(modList) { mod in
                    HStack {
                        Text(mod.name)
                        Text(mod.desc)
                    }
                }
            }
            .navigationTitle("Mods")
        }
        .padding()
        .onAppear(perform: initializeSDK)
        .onDisappear(perform: ModioSDKHelper.shutdownModioSDK)
    }
    
    func initializeSDK()
    {
        // Start the mod.io SDK
        ModioSDKHelper.startModioSDK({ didFinish in
            if (didFinish == true)
            {
                // Once it has finished succesfully, then other queries
                // can be fulfilled
                loadList()
            }
        });
    }
    
    func loadList()
    {
        ModioSDKHelper.modioModList({ (list: [Any]) -> Void in
            let mods = list as NSArray;
            for mod in mods
            {
                // Parse code from ObjetiveC to a Swift struct
                guard let modDic = mod as? NSDictionary,
                      let modID = modDic["ModID"] as? NSNumber,
                      let profileName = modDic["ProfileName"] as? String,
                      var profileDesc = modDic["ProfileDescription"] as? String
                else {
                    continue
                }
                
                let maxLength = 25
                if (profileDesc.count > maxLength)
                {
                    let start = profileDesc.index(profileDesc.startIndex, offsetBy: maxLength)
                    let range = start ..< profileDesc.endIndex
                    profileDesc.removeSubrange(range)
                }
                
                let modInfo = ModInfo(id: ObjectIdentifier(modID), name: profileName, desc: profileDesc)
                self.modList.append(modInfo)
            }
        })
    }
}
