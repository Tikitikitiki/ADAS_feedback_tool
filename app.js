const eventsKey = 'adas_events'
let map
let markers = []

function setStatus(msg){
  const el = document.getElementById('status')
  if(el) el.textContent = msg
}

function loadEvents(){
  try{ return JSON.parse(localStorage.getItem(eventsKey)||'[]') }catch(e){return []}
}

function saveEvents(list){
  localStorage.setItem(eventsKey, JSON.stringify(list))
}

function initMap(){
  const mapElement = document.getElementById('map')
  if(!mapElement) return
  
  map = L.map(mapElement).setView([37.7749, -122.4194], 12)
  
  L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '© OpenStreetMap contributors',
    maxZoom: 19
  }).addTo(map)
}

function updateMap(){
  if(!map) return
  
  markers.forEach(m => map.removeLayer(m))
  markers = []
  
  const list = loadEvents()
  const group = new L.featureGroup()
  
  list.forEach(ev => {
    if(ev.lat && ev.lng){
      const marker = L.circleMarker([ev.lat, ev.lng], {
        radius: 6,
        fillColor: '#ff4444',
        color: '#cc0000',
        weight: 2,
        opacity: 0.8,
        fillOpacity: 0.7
      })
      
      marker.bindPopup(`<div style="font-size:12px"><strong>${ev.type}</strong><br>${ev.time}</div>`)
      
      marker.addTo(map)
      markers.push(marker)
      group.addLayer(marker)
    }
  })
  
  if(markers.length > 0){
    map.fitBounds(group.getBounds(), {padding: [50, 50]})
  }
}

function render(){
  const list = loadEvents()
  const ul = document.getElementById('eventsList')
  ul.innerHTML = ''
  list.slice().reverse().forEach(ev=>{
    const li = document.createElement('li')
    li.textContent = `${ev.type} — ${ev.time} ${ev.lat?`(lat:${ev.lat.toFixed(5)} lng:${ev.lng.toFixed(5)})`:"(no-geo)"}`
    ul.appendChild(li)
  })
  updateMap()
}

function recordEvent(type){
  setStatus('Getting location...')
  const base = {type, time: new Date().toISOString(), lat:null, lng:null}

  const finish = (data)=>{
    const list = loadEvents()
    list.push(Object.assign({}, base, data))
    saveEvents(list)
    render()
    setStatus('Saved: '+type)
  }

  if(navigator.geolocation){
    navigator.geolocation.getCurrentPosition(pos=>{
      finish({lat:pos.coords.latitude, lng:pos.coords.longitude})
    },err=>{
      finish({})
    },{maximumAge:30000, timeout:8000})
  }else{
    finish({})
  }
}

function downloadCSV(){
  const list = loadEvents()
  if(!list.length){setStatus('No events to export');return}
  
  const rows = [['Type','Timestamp','Latitude','Longitude']]
  list.forEach(ev=>{
    rows.push([ev.type, ev.time, ev.lat||'', ev.lng||''])
  })
  
  const csv = rows.map(r=>r.map(cell=>`"${(cell+'').replace(/"/g,'""')}"`).join(',')).join('\n')
  const blob = new Blob([csv], {type:'text/csv;charset=utf-8;'})
  const url = URL.createObjectURL(blob)
  const a = document.createElement('a')
  a.href = url
  a.download = `adas-events-${new Date().toISOString().slice(0,19).replace(/:/g,'-')}.csv`
  document.body.appendChild(a)
  a.click()
  document.body.removeChild(a)
  URL.revokeObjectURL(url)
  setStatus('CSV exported')
}

document.addEventListener('DOMContentLoaded',()=>{
  initMap()
  document.querySelectorAll('button.event').forEach(b=>{
    b.addEventListener('click',e=>{
      const t = b.getAttribute('data-event')
      recordEvent(t)
    })
  })
  const exportBtn = document.getElementById('exportBtn')
  if(exportBtn) exportBtn.addEventListener('click',downloadCSV)
  
  const zoomInBtn = document.getElementById('zoomInBtn')
  const zoomOutBtn = document.getElementById('zoomOutBtn')
  if(zoomInBtn) zoomInBtn.addEventListener('click',()=>{
    if(map) map.setZoom(map.getZoom() + 1)
  })
  if(zoomOutBtn) zoomOutBtn.addEventListener('click',()=>{
    if(map) map.setZoom(map.getZoom() - 1)
  })
  
  render()
})
