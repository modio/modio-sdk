"use strict";(self.webpackChunkmodio_docs=self.webpackChunkmodio_docs||[]).push([[6429],{4058:(e,i,t)=>{t.r(i),t.d(i,{assets:()=>a,contentTitle:()=>d,default:()=>p,frontMatter:()=>s,metadata:()=>r,toc:()=>c});var o=t(4848),n=t(8453);const s={id:"ios-getting-started",title:"Getting Started",slug:"/cppsdk/ios/getting-started",sidebar_position:0,custom_edit_url:"https://github.com/modio/modio-sdk-internal/blob/develop/platform/ios/doc/getting-started.mdx"},d="Getting Started",r={id:"game-integration/cppsdk/ios/ios-getting-started",title:"Getting Started",description:"mod.io iOS SDK Integration",source:"@site/public/en-us/game-integration/cppsdk/ios/getting-started.mdx",sourceDirName:"game-integration/cppsdk/ios",slug:"/cppsdk/ios/getting-started",permalink:"/cppsdk/ios/getting-started",draft:!1,unlisted:!1,editUrl:"https://github.com/modio/modio-sdk-internal/blob/develop/platform/ios/doc/getting-started.mdx",tags:[],version:"current",sidebarPosition:0,frontMatter:{id:"ios-getting-started",title:"Getting Started",slug:"/cppsdk/ios/getting-started",sidebar_position:0,custom_edit_url:"https://github.com/modio/modio-sdk-internal/blob/develop/platform/ios/doc/getting-started.mdx"},sidebar:"sidebar",previous:{title:"CMake Integration",permalink:"/cppsdk/mac/cmake-integration"},next:{title:"CMake Integration",permalink:"/cppsdk/ios/cmake-integration"}},a={},c=[{value:"mod.io iOS SDK Integration",id:"modio-ios-sdk-integration",level:2},{value:"iOS SDK Versions",id:"ios-sdk-versions",level:2},{value:"iOS Device Provisioning profiles",id:"ios-device-provisioning-profiles",level:2}];function l(e){const i={a:"a",code:"code",h1:"h1",h2:"h2",p:"p",pre:"pre",...(0,n.R)(),...e.components};return(0,o.jsxs)(o.Fragment,{children:[(0,o.jsx)(i.h1,{id:"getting-started",children:"Getting Started"}),"\n",(0,o.jsx)(i.h2,{id:"modio-ios-sdk-integration",children:"mod.io iOS SDK Integration"}),"\n",(0,o.jsxs)(i.p,{children:["The use of iOS with the mod.io SDK is ready by default. You can find the ",(0,o.jsx)(i.code,{children:"ios"})," directory in the platform subdirectory of your SDK installation.\nYou can then target ",(0,o.jsx)(i.code,{children:"IOS"})," to compile the mod.io SDK for iOS SDK."]}),"\n",(0,o.jsx)(i.h2,{id:"ios-sdk-versions",children:"iOS SDK Versions"}),"\n",(0,o.jsx)(i.p,{children:"The mod.io SDK is built and tested against iOS using XCode v14 & v15.1. Using different versions of the iOS SDK may result in compilation issues."}),"\n",(0,o.jsxs)(i.p,{children:["In iOS, the default way to compile the mod.io SDK requires CMake, which you can install following the ",(0,o.jsx)(i.a,{href:"https://cmake.org/download/",children:"CMake"})," instructions or using a package manager like ",(0,o.jsx)(i.code,{children:"brew"}),":"]}),"\n",(0,o.jsx)(i.pre,{children:(0,o.jsx)(i.code,{className:"language-cmake",children:"brew install cmake ninja\n"})}),"\n",(0,o.jsxs)(i.p,{children:["NOTE: You can use Ninja or XCode as project generators. The former works like any other C++ project. The later will create a ",(0,o.jsx)(i.code,{children:".xcodeproj"})," file in the designated output folder."]}),"\n",(0,o.jsxs)(i.p,{children:["iOS compiles by default to the simulator. If you need to deploy to a device, add variable ",(0,o.jsx)(i.code,{children:"MODIO_IOS_DEVICE"})," to cmake. Check documentation in the platform link",":platform","/ios/README.adoc[README.adoc] for more details."]}),"\n",(0,o.jsx)(i.p,{children:"The minimum deployment target is iOS 15.0"}),"\n",(0,o.jsx)(i.h2,{id:"ios-device-provisioning-profiles",children:"iOS Device Provisioning profiles"}),"\n",(0,o.jsxs)(i.p,{children:["The mod.io SDK in iOS compiles by default to the simulator. If you need to deploy to a device, add variable ",(0,o.jsx)(i.code,{children:"MODIO_IOS_DEVICE"})," to cmake, for example:"]}),"\n",(0,o.jsx)(i.pre,{children:(0,o.jsx)(i.code,{className:"language-cmake",children:"cmake -G Xcode -DMODIO_IOS_DEVICE:BOOL=true -DMODIO_PLATFORM:STRING=IOS -B out\n"})}),"\n",(0,o.jsxs)(i.p,{children:["Using Xcode as a generator, will create the file ",(0,o.jsx)(i.code,{children:"out/modio.xcodeproj"})," which is used to organize and compile the project. To deploy to a device, you need a developer certificate signed by Apple and a provisioning profile that includes at least one target device identified associated to it."]}),"\n",(0,o.jsxs)(i.p,{children:["Check ",(0,o.jsx)(i.a,{href:"https://developer.apple.com/documentation/xcode/devices-and-simulator",children:"Apple documentation"})," to retrieve those elements."]}),"\n",(0,o.jsxs)(i.p,{children:["When you have those, update the ",(0,o.jsx)(i.code,{children:"Signing And Capabilities"})," section in Xcode with your corresponding ",(0,o.jsx)(i.code,{children:"Bundle Identifier"}),", ",(0,o.jsx)(i.code,{children:"Team"})," and ",(0,o.jsx)(i.code,{children:"Provisioning Profile"}),"."]})]})}function p(e={}){const{wrapper:i}={...(0,n.R)(),...e.components};return i?(0,o.jsx)(i,{...e,children:(0,o.jsx)(l,{...e})}):l(e)}},8453:(e,i,t)=>{t.d(i,{R:()=>d,x:()=>r});var o=t(6540);const n={},s=o.createContext(n);function d(e){const i=o.useContext(s);return o.useMemo((function(){return"function"==typeof e?e(i):{...i,...e}}),[i,e])}function r(e){let i;return i=e.disableParentContext?"function"==typeof e.components?e.components(n):e.components||n:d(e.components),o.createElement(s.Provider,{value:i},e.children)}}}]);