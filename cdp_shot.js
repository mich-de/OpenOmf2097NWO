// node cdp_shot.js <url> <waitMs> <outPngPath> [clickX,clickY] [waitAfterClickMs]
const fs = require('fs');
const [, , url, waitMsArg, outPath, clickArg, waitAfterClickArg] = process.argv;
const waitMs = Number(waitMsArg);
const click = clickArg ? clickArg.split(',').map(Number) : null;
const waitAfterClickMs = Number(waitAfterClickArg || 0);

async function main() {
    const targets = await (await fetch('http://127.0.0.1:9333/json/list')).json();
    let target = targets.find(t => t.type === 'page');
    if (!target) target = await (await fetch('http://127.0.0.1:9333/json/new')).json();
    const ws = new WebSocket(target.webSocketDebuggerUrl);
    let id = 0;
    const pending = new Map();
    const events = [];
    ws.addEventListener('message', ev => {
        const msg = JSON.parse(ev.data);
        if (msg.id !== undefined && pending.has(msg.id)) {
            pending.get(msg.id)(msg);
            pending.delete(msg.id);
        } else if (msg.method) {
            events.push(msg);
        }
    });
    function send(method, params = {}) {
        const myId = ++id;
        return new Promise(resolve => {
            pending.set(myId, resolve);
            ws.send(JSON.stringify({ id: myId, method, params }));
        });
    }
    await new Promise(r => ws.addEventListener('open', r));
    console.log('enable:', JSON.stringify(await send('Page.enable')));
    console.log('enable:', JSON.stringify(await send('Runtime.enable')));
    console.log('enable:', JSON.stringify(await send('Log.enable')));
    console.log('navigate:', JSON.stringify(await send('Page.navigate', { url })));
    await new Promise(r => setTimeout(r, waitMs));

    if (click) {
        const [x, y] = click;
        await send('Input.dispatchMouseEvent', { type: 'mousePressed', x, y, button: 'left', clickCount: 1 });
        await send('Input.dispatchMouseEvent', { type: 'mouseReleased', x, y, button: 'left', clickCount: 1 });
        console.log(`clicked (${x},${y})`);
        await new Promise(r => setTimeout(r, waitAfterClickMs));
    }

    const logText = await send('Runtime.evaluate', { expression: 'JSON.stringify(window.testLog || [])', returnByValue: true });
    console.log('testLog raw:', JSON.stringify(logText));

    const consoleEvents = events.filter(e => ['Runtime.consoleAPICalled', 'Log.entryAdded', 'Runtime.exceptionThrown'].includes(e.method));
    for (const e of consoleEvents.slice(-20)) {
        const p = e.params;
        if (p.args) console.log('[console]', p.args.map(a => a.value ?? a.description).join(' '));
        else if (p.entry) console.log('[log]', p.entry.level, p.entry.text);
        else if (p.exceptionDetails) console.log('[exception]', p.exceptionDetails.exception?.description || p.exceptionDetails.text);
    }
    if (process.env.EVAL) {
        const r = await send('Runtime.evaluate', { expression: process.env.EVAL, returnByValue: true });
        console.log('[eval]', JSON.stringify(r.result?.result?.value ?? r.result?.exceptionDetails?.text ?? r));
    }

    const shot = await send('Page.captureScreenshot', { format: 'png' });
    if (!shot.result) { console.log('screenshot error:', JSON.stringify(shot)); }
    else { fs.writeFileSync(outPath, Buffer.from(shot.result.data, 'base64')); console.log('saved', outPath); }
    ws.close();
}
main().then(() => process.exit(0)).catch(e => { console.error('FATAL', e); process.exit(1); });
