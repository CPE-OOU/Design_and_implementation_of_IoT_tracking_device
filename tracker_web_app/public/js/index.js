///-------firebase imports and configurations------///
import { initializeApp } from "https://www.gstatic.com/firebasejs/9.21.0/firebase-app.js";
  import { getDatabase, ref, onValue} from "https://www.gstatic.com/firebasejs/9.21.0/firebase-database.js";
  
  const firebaseConfig = {
    apiKey: "AIzaSyBXGoVyO55Ah2iELOFCd6g9LjnRXz_RyQM",
    authDomain: "tracker-e1965.firebaseapp.com",
    databaseURL: "https://tracker-e1965-default-rtdb.firebaseio.com",
    projectId: "tracker-e1965",
    storageBucket: "tracker-e1965.appspot.com",
    messagingSenderId: "733393261217",
    appId: "1:733393261217:web:712e3e0c63a2fb048f56e0",
    measurementId: "G-ZMLZ88GTQ7"
  };

  // Initialize Firebase
  const app = initializeApp(firebaseConfig);
  
  const db = getDatabase();
   const reference =ref(db,"/");

  onValue(reference, (snapshot)=>{
   var data = snapshot.val();
   document.getElementById("lat-value").innerHTML=data.lat
   document.getElementById("long-value").innerHTML=data.lng
   console.log(data);

   
   var lat = data.lat
   var lon = data.lng
  var latlon = new google.maps.LatLng(lat, lon)
 var mapholder = document.getElementById('mapholder')
    mapholder.style.height = '300px';
   mapholder.style.width = '495px';
 
  var myOptions = {
 
    center:latlon,zoom:18,   
     mapTypeId:google.maps.MapTypeId.ROADMAP
  }
  
  var map = new google.maps.Map(document.getElementById("mapholder"), myOptions);
  const marker = new google.maps.Marker({position:latlon,map:map,title:"You are here!"});
  
  //  let button = document.getElementById('click-button');
  //  button.addEventListener('click', ()=>{
  //    window.open('http://maps.google.com/maps?q=loc:6.80,3.11', '_blank');
  //  });
   
  }

   
  )